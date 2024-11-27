'''
Module storing LogInfo class
'''
import os
import re
import glob
import zipfile
import functools

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('ap_utilities:log_info')
# ---------------------------------------------
class LogInfo:
    '''
    Class taking a zip file with logging information from AP pipelines 
    and extracting information like the number of entries that it ran over
    '''
    # ---------------------------------------------
    def __init__(self, zip_path : str):
        self._zip_path = zip_path
        self._out_path = '/tmp/log_info'
        self._log_wc   = 'DaVinci_*.log'
        self._log_path : str

        self._entries_regex : str = r'\s*\|\s*"#\snon-empty events for field .*"\s*\|\s*(\d+)\s*\|.*'

        os.makedirs(self._out_path, exist_ok=True)
    # ---------------------------------------------
    def _get_log_path(self) -> str:
        path_wc = f'{self._out_path}/*/{self._log_wc}'

        try:
            [log_path] = glob.glob(path_wc)
        except ValueError as exc:
            raise FileNotFoundError(f'Cannot find one and only one DaVinci log file in: {path_wc}') from exc

        return log_path
    # ---------------------------------------------
    @functools.lru_cache()
    def _get_dv_lines(self) -> list[str]:
        with zipfile.ZipFile(self._zip_path, 'r') as zip_ref:
            zip_ref.extractall(self._out_path)

        self._log_path = self._get_log_path()

        with open(self._log_path, encoding='utf-8') as ifile:
            l_line = ifile.read().splitlines()

        return l_line
    # ---------------------------------------------
    def _entries_from_line(self, line : str) -> int:
        mtch = re.match(self._entries_regex, line)
        if not mtch:
            raise ValueError(f'Cannot extract number of entries from line \"{line}\" using regex \"{self._entries_regex}\"')

        entries = mtch.group(1)

        return int(entries)
    # ---------------------------------------------
    def _get_line_with_entries(self, l_line : list[str], alg_name : str) -> str:
        algo_index = None
        for i_line, line in enumerate(l_line):
            if alg_name in line and 'Number of counters' in line:
                algo_index = i_line
                break

        if algo_index is None:
            raise ValueError(f'Cannot find line with \"Number of counters\" and \"{alg_name}\" in {self._log_path}')

        return l_line[algo_index + 2]
    # ---------------------------------------------
    def get_mcdt_entries(self, alg_name : str) -> int:
        '''
        Returns entries that DaVinci ran over to get MCDecayTree
        '''
        l_line            = self._get_dv_lines()
        line_with_entries = self._get_line_with_entries(l_line, alg_name)
        nentries          = self._entries_from_line(line_with_entries)

        log.debug(f'Found {nentries} entries')

        return nentries
# ---------------------------------------------
