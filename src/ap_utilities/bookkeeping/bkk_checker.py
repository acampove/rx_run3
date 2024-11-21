'''
Module with BkkChecker class
'''

import re
from concurrent.futures     import ThreadPoolExecutor

import subprocess
import yaml
from dmu.logging.log_store  import LogStore

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
            self._d_cfg        = yaml.safe_load(ifile)
            self._d_event_type = self._d_cfg['event_type']

        self._input_path   : str = path
        self._year         : str = self._d_cfg['settings']['year']
        self._mc_path      : str = self._d_cfg['settings']['mc_path']
        self._nu_path      : str = self._d_cfg['settings']['nu_path']
        self._polarity     : str = self._d_cfg['settings']['polarity']
        self._generator    : str = self._d_cfg['settings']['generator']
        self._sim_version  : str = self._d_cfg['settings']['sim_vers']
        self._ctags        : str = self._d_cfg['settings']['ctags']
        self._dtags        : str = self._d_cfg['settings']['dtags']
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
        sample_path = f'/MC/{self._year}/Beam6800GeV-{self._mc_path}-{self._polarity}-{self._nu_path}-25ns-{self._generator}/{self._sim_version}/HLT2-{self._mc_path}/{event_type}/DST'

        log.debug(f'{"":<4}{sample_path:<100}')

        cmd_bkk = ['dirac-bookkeeping-get-stats', '-B' , sample_path]
        result  = subprocess.run(cmd_bkk, capture_output=True, text=True, check=False)
        nfile   = self._nfiles_from_stdout(result.stdout)

        return nfile != 0
    # -------------------------
    def _get_samples_with_threads(self, nthreads : int) -> dict[str,str]:
        l_found : list[bool] = []
        with ThreadPoolExecutor(max_workers=nthreads) as executor:
            l_result = [ executor.submit(self._was_found, event_type) for event_type in self._d_event_type.values() ]
            l_found  = [result.result() for result in l_result ]

        d_event_type = {}
        for nick_name, found in zip(self._d_event_type, l_found):
            if not found:
                continue

            d_event_type[nick_name] = self._d_event_type[nick_name]

        return d_event_type
    # -------------------------
    def _save_to_text(self, d_event_type : dict[str,str]) -> None:
        text = ''
        for nick_name, evt_type in d_event_type.items():
            nu_name = self._nu_path.replace('.', 'p')
            text   += f'("{nick_name}", "{evt_type}" , "{self._mc_path}", "{self._polarity}"  , "{self._ctags}", "{self._dtags}", "{self._nu_path}", "{nu_name}", "{self._sim_version}", "{self._generator}" ),\n'

        output_path = self._input_path.replace('.yaml', '.txt')

        log.info(f'Saving to: {output_path}')
        with open(output_path, 'w', encoding='utf-8') as ofile:
            ofile.write(text)
    # -------------------------
    def save(self, nthreads : int = 1) -> None:
        '''
        Will check if samples exist in grid
        Will save list of found samples to text file with same name as input YAML, but with txt extension
        '''

        log.info('Filtering input')
        if nthreads == 1:
            log.info('Using single thread')
            d_event_type = { nick_name : event_type for nick_name, event_type in self._d_event_type.items() if self._was_found(event_type) }
        else:
            log.info(f'Using {nthreads} threads')
            d_event_type = self._get_samples_with_threads(nthreads)

        nfound = len(d_event_type)
        npased = len(self._d_event_type)

        log.info(f'Found: {nfound}/{npased}')
        self._save_to_text(d_event_type)
# ---------------------------------
