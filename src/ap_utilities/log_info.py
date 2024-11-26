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

        self._entries_regex : str = r'\s*\|\s*Sum of all Algorithms\s*\|\s*(\d+)\s*\|.*'

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
    def get_ran_entries(self) -> int:
        '''
        Returns entries that DaVinci ran over
        '''
        l_line    = self._get_dv_lines()
        l_entries = [ line for line in l_line if 'Sum of all Algorithms' in line ]
        if len(l_entries) != 1:
            raise ValueError(f'Not one and only one line with sum of all algorithms found in {self._log_path}')

        nentries = self._entries_from_line(l_entries[0])

        log.info(f'Found {nentries} entries')

        return nentries
# ---------------------------------------------
