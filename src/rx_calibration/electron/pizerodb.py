'''
Module holding PiZeroDb class
'''

from importlib.resources import files

import pandas as pnd
from dmu.logging.log_store   import LogStore

log=LogStore.add_logger('rx_calibration:pizerodb')
# ---------------------------------------
class PiZeroDb:
    '''
    Class in charge of:

    - Read CSV file with run number, start date, taken from:
    https://lbrundb.cern.ch/rundb/
    - For each run number passed return number of days since last pi0 calibration
    '''
    # -----------------------------
    def __init__(self):
        '''
        No args
        '''

        file_path = files('rx_calibration_data').joinpath('ecal_calib/rundb.csv')
        file_path = str(file_path)
        self._df  = pnd.read_csv(file_path)
    # -----------------------------
    def get_period(self, run : int) -> int:
        return 3
# ---------------------------------------
