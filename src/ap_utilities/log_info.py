'''
Module storing LogInfo class
'''
import os
import glob
import zipfile

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
    def _get_lines(self) -> list[str]:
        with zipfile.ZipFile(self._zip_path, 'r') as zip_ref:
            zip_ref.extractall(self._out_path)

        log_path = self._get_log_path()

        with open(log_path, encoding='utf-8') as ifile:
            l_line = ifile.read().splitlines()

        return l_line
    # ---------------------------------------------
    def get_ran_entries(self) -> int:
        '''
        Returns entries that DaVinci ran over
        '''
        l_line = self._get_lines()
# ---------------------------------------------
