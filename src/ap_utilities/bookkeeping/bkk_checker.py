'''
Module with BkkChecker class
'''

import re
import os
from concurrent.futures     import ThreadPoolExecutor

import subprocess
import yaml

import ap_utilities.decays.utilities as aput
from ap_utilities.logging.log_store  import LogStore

log=LogStore.add_logger('ap_utilities:Bookkeeping.bkk_checker')
# ---------------------------------
class BkkChecker:
    '''
    Class meant to check if samples exist in Bookkeeping using multithreading.
    This is useful with large lists of samples, due to low performance of Dirac
    '''
    # pylint: disable=too-few-public-methods
    # -------------------------
    def __init__(self, path : str):
        '''
        Takes the path to a YAML file with the list of samples
        '''
        with open(path, encoding='utf-8') as ifile:
            self._d_cfg                           = yaml.safe_load(ifile)
            self._l_event_type_single : list[str] = self._get_types('event_type')
            self._l_event_type_double : list[str] = self._get_types('event_type_split_sim')

            self._l_event_type = self._l_event_type_single + self._l_event_type_double

        self._input_path   : str = path
        self._year         : str = self._d_cfg['settings']['year']
        self._mc_path      : str = self._d_cfg['settings']['mc_path']
        self._nu_path      : str = self._d_cfg['settings']['nu_path']
        self._polarity     : str = self._d_cfg['settings']['polarity']
        self._generator    : str = self._d_cfg['settings']['generator']
        self._sim_version  : str = self._d_cfg['settings']['sim_vers']
        self._ctags        : str = self._d_cfg['settings']['ctags']
        self._dtags        : str = self._d_cfg['settings']['dtags']

        # For split-sim samples the sim substring looks like Sim10d-SplitSim02
        self._split_sim_suffix = 'SplitSim02'
    # -------------------------
    def _get_types(self, kind : str) -> list[str]:
        if kind not in self._d_cfg:
            raise ValueError(f'Cannot find kind of event type: {kind}')

        l_event_type = self._d_cfg[kind]
        s_event_type = set(l_event_type)

        if len(l_event_type) != len(s_event_type):
            raise ValueError('Duplicate event types found')

        return l_event_type
    # -------------------------
    def _nfiles_line_from_stdout(self, stdout : str) -> str:
        l_line = stdout.split('\n')
        try:
            [line] = [ line for line in l_line if line.startswith('Nb of Files') ]
        except ValueError:
            log.warning(f'Cannot find number of files in: \n{stdout}')
            return 'None'

        return line
    # -------------------------
    def _nfiles_from_stdout(self, stdout : str) -> int:
        line  = self._nfiles_line_from_stdout(stdout)
        log.debug(f'Searching in line {line}')

        regex = r'Nb of Files      :  (\d+|None)'
        mtch  = re.match(regex, line)

        if not mtch:
            raise ValueError(f'No match found in: \n{stdout}')

        nsample = mtch.group(1)
        if nsample == 'None':
            log.debug('Found zero files')
            return 0

        log.debug(f'Found {nsample} files')

        return int(nsample)
    # -------------------------
    def _was_found(self, event_type : str) -> bool:
        bkk_simple = self._get_bkk(event_type, is_split_sim=False)
        found      = self._find_bkk(bkk_simple)

        if event_type not in self._l_event_type_double:
            return found

        bkk_split  = self._get_bkk(event_type, is_split_sim=True)
        found_ss   = self._find_bkk(bkk_split)

        # Event type will only be found, if both split sim and normal samples are found

        return found and found_ss
    # -------------------------
    def _get_bkk(self, event_type : str, is_split_sim : bool) -> str:
        sim_name    = self._sim_version if not is_split_sim else f'{self._sim_version}-{self._split_sim_suffix}'
        sample_path = f'/MC/{self._year}/Beam6800GeV-{self._mc_path}-{self._polarity}-{self._nu_path}-25ns-{self._generator}/{sim_name}/HLT2-{self._mc_path}/{event_type}/DST'

        log.debug(f'{"":<4}{sample_path:<100}')

        return sample_path
    # -------------------------
    def _find_bkk(self, bkk : str) -> bool:
        cmd_bkk = ['dirac-bookkeeping-get-stats', '-B' , bkk]
        result  = subprocess.run(cmd_bkk, capture_output=True, text=True, check=False)
        nfile   = self._nfiles_from_stdout(result.stdout)
        found   = nfile != 0

        return found
    # -------------------------
    def _get_samples_with_threads(self, nthreads : int) -> list[str]:
        l_found : list[bool] = []
        with ThreadPoolExecutor(max_workers=nthreads) as executor:
            l_result = [ executor.submit(self._was_found, event_type) for event_type in self._l_event_type ]
            l_found  = [result.result() for result in l_result ]

        l_event_type = [ event_type for event_type, found in zip(self._l_event_type, l_found) if found ]

        return l_event_type
    # -------------------------
    def _save_info_yaml(self, l_event_type : list[str]) -> None:
        text = ''
        for evt_type in l_event_type:
            nu_name         = self._nu_path.replace('.', 'p')
            nick_name_org   = aput.read_decay_name(evt_type, style='safe_1')
            sim_version     = f'"{self._sim_version}"'
            nick_name       = f'"{nick_name_org}"'
            text           += f'({nick_name:<60}, "{evt_type}" , "{self._mc_path}", "{self._polarity}"  , "{self._ctags}", "{self._dtags}", "{self._nu_path}", "{nu_name}", {sim_version:<20}, "{self._generator}" ),\n'

            if evt_type in self._l_event_type_double:
                nick_name   = f'"{nick_name_org}_SS"'
                sim_version = f'"{self._sim_version}-{self._split_sim_suffix}"'
                text       += f'({nick_name:<60}, "{evt_type}" , "{self._mc_path}", "{self._polarity}"  , "{self._ctags}", "{self._dtags}", "{self._nu_path}", "{nu_name}", {sim_version:<20}, "{self._generator}" ),\n'

        output_dir  = os.path.dirname(self._input_path)
        output_path = f'{output_dir}/info.yaml'

        log.info(f'Saving to: {output_path}')
        with open(output_path, 'w', encoding='utf-8') as ofile:
            ofile.write(text)
    # -------------------------
    def _save_validation_config(self, l_event_type : list[str]) -> None:
        d_data = {'samples' : {}}
        for event_type in l_event_type:
            nick_name = aput.read_decay_name(event_type, style='safe_1')
            d_data['samples'][nick_name] = ['any']

        output_dir  = os.path.dirname(self._input_path)
        output_path = f'{output_dir}/validation.yaml'

        log.info(f'Saving to: {output_path}')
        with open(output_path, 'w', encoding='utf-8') as ofile:
            yaml.safe_dump(d_data, ofile, width=200)
    # -------------------------
    def save(self, nthreads : int = 1) -> None:
        '''
        Will check if samples exist in grid
        Will save list of found samples to text file with same name as input YAML, but with txt extension
        '''

        log.info('Filtering input')
        if nthreads == 1:
            log.info('Using single thread')
            l_event_type = [ event_type for event_type in self._l_event_type if self._was_found(event_type) ]
        else:
            log.info(f'Using {nthreads} threads')
            l_event_type = self._get_samples_with_threads(nthreads)

        nfound = len(l_event_type)
        npased = len(self._l_event_type)

        log.info(f'Found: {nfound}/{npased}')
        self._save_info_yaml(l_event_type)
        self._save_validation_config(l_event_type)
# ---------------------------------
