'''
Module holding PiZeroDb class
'''
import bisect
from datetime            import datetime as DateTime
from importlib.resources import files

import pandas as pnd
from dmu.logging.log_store   import LogStore
from dmu.generic             import utilities as gut

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
        log.debug('Initializing PiZeroDb')

        self._df_run = self._load_df(name='rundb')
        self._l_cal  = self._get_calibration_dates()
    # -----------------------------
    def _get_calibration_dates(self) -> list[DateTime]:
        df      = self._load_df(name='pi0')
        sr_date = df['pizero_calibrations']
        sr_date = pnd.to_datetime(sr_date)

        return sorted(sr_date.tolist())
    # -----------------------------
    def _load_df(self, name : str) -> pnd.DataFrame:
        file_path = files('rx_calibration_data').joinpath(f'ecal_calib/{name}.yaml')
        file_path = str(file_path)
        data      = gut.load_json(file_path)
        df        = pnd.DataFrame(data)

        if name == 'rundb':
            df = df.set_index('runid', drop=True)

        return df
    # -----------------------------
    def _find_date(self, run : int) -> DateTime:
        date_str = self._df_run['starttime'].get(run, None)
        if date_str is None:
            raise ValueError(f'No entry with date found for run: {run}')

        date_str = str(date_str)
        log.debug(f'Found date {date_str} for run {run}')

        date     = DateTime.fromisoformat(date_str)

        return date
    # -----------------------------
    def get_period(self, run : int) -> int:
        '''
        Takes the run number and returns the number of days since last
        pi0 calibration
        '''
        run_date = self._find_date(run=run)
        index    = bisect.bisect_left(self._l_cal, run_date)
        if index == 0:
            log.debug(f'No calibration was performed before run: {run}')
            return -1

        cal_date = self._l_cal[index - 1]

        log.debug(f'Calibration at: {cal_date}')
        log.debug(f'Run         at: {run_date}')

        # Measure time passed since LAST calibration
        assert run_date > cal_date

        return (run_date - cal_date).days
# ---------------------------------------
