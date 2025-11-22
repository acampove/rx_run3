'''
This module is used by pytest to _inject_ fixtures in the tests
'''
import os
import logging
from importlib.resources import files

import matplotlib.pyplot as plt
import pytest
import pandas as pnd

from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

# ---------------------------------------
class ConfData:
    '''
    Class used to hold data needed for tests
    '''
    l_run : list[int]

    df_pi0= pnd.DataFrame(columns=['run', 'period'])

    @staticmethod
    def add_run_period(run : int, period : int) -> None:
        '''
        Adds to dataframe the run number and the number of days since
        last pi0 calibration
        '''
        df = ConfData.df_pi0

        df.loc[len(df)] = [run, period]
# ---------------------------------------
def _load_runs() -> None:
    df = _load_df(name='rundb')

    ConfData.l_run = df['runid'].tolist()
# -----------------------------
def _load_df(name : str) -> pnd.DataFrame:
    file_path = files('rx_calibration_data').joinpath(f'ecal_calib/{name}.yaml')
    file_path = str(file_path)
    data      = gut.load_json(file_path)
    df        = pnd.DataFrame(data)

    return df
# ---------------------------------------
def pytest_configure(config : pytest.Config) -> None:
    '''
    Runs before all tests, needed to do global initialization. e.g. logging level setting
    '''
    _ = config

    logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)
    logging.getLogger("matplotlib.font_manager").setLevel(logging.WARNING)

    LogStore.set_level('rx_calibration:pizerodb', 20)

    _load_runs()
# ---------------------------------------
def _save_run_period() -> None:
    df      = ConfData.df_pi0
    out_dir = '/tmp/tests/rx_calibration/pizerodb'
    os.makedirs(out_dir, exist_ok=True)

    df.plot.scatter('run', 'period', s=1)
    plt.savefig(f'{out_dir}/run_period.png')
    plt.close()
# ---------------------------------------
def pytest_sessionfinish(session, exitstatus):
    _save_run_period()
# ---------------------------------------
