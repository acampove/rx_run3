'''
Module with BkkChecker class
'''

import re
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
    def __init__(self, name : str, d_section : dict):
        '''
        Takes:

        name     : Name of section, needed to dump output
        d_Section: A dictionary representing sections of samples
        '''

        self._suffix       : str = '' if 'suffix' not in d_section else d_section['suffix']
        self._name         : str = name

        self._year         : str = d_section['settings']['year']
        self._mc_path      : str = d_section['settings']['mc_path']
        self._nu_path      : str = d_section['settings']['nu_path']
        self._polarity     : str = d_section['settings']['polarity']
        self._generator    : str = d_section['settings']['generator']
        self._sim_version  : str = d_section['settings']['sim_vers']
        self._ctags        : str = d_section['settings']['ctags']
        self._dtags        : str = d_section['settings']['dtags']

        self._l_event_type : list[str] = self._get_event_types(d_section)
    # -------------------------
    def _get_event_types(self, d_section : dict) -> list[str]:
        l_evt  = self._list_from_dict(d_section, 'evt_type')
        nevt = len(l_evt)
        log.debug(f'Found {nevt} event types')

        l_nick = self._list_from_dict(d_section, 'nickname')
        nnick = len(l_nick)
        log.debug(f'Found {nnick} nicknames')

        l_evt += [ aput.read_event_type(nick) for nick in l_nick ]

        return l_evt
    # -------------------------
    def _list_from_dict(self, d_section : dict, key : str) -> list[str]:
        if key not in d_section:
            return []

        return d_section[key]
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
        bkk   = self._get_bkk(event_type)
        found = self._find_bkk(bkk)

        return found
    # -------------------------
    def _get_bkk(self, event_type : str) -> str:
        sample_path = f'/MC/{self._year}/Beam6800GeV-{self._mc_path}-{self._polarity}-{self._nu_path}-25ns-{self._generator}/{self._sim_version}/HLT2-{self._mc_path}/{event_type}/DST'

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
            nick_name       = f'"{nick_name_org}{self._suffix}"'
            sim_vers        = f'"{self._sim_version}"'
            text           += f'({nick_name:<60}, "{evt_type}" , "{self._mc_path}", "{self._polarity}"  , "{self._ctags}", "{self._dtags}", "{self._nu_path}", "{nu_name}", {sim_vers:<20}, "{self._generator}" ),\n'

        output_path = f'info_{self._name}.yaml'
        log.info(f'Saving to: {output_path}')
        with open(output_path, 'w', encoding='utf-8') as ofile:
            ofile.write(text)
    # -------------------------
    def _save_validation_config(self, l_event_type : list[str]) -> None:
        d_data = {'samples' : {}}
        for event_type in l_event_type:
            nick_name = aput.read_decay_name(event_type, style='safe_1')
            d_data['samples'][nick_name] = ['any']

        output_path = f'validation_{self._name}.yaml'
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
