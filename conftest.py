'''
This module is used by pytest to _inject_ fixtures in the tests
'''
import logging
from importlib.resources import files

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

    if name == 'rundb':
        df = df.set_index('runid', drop=True)

    return df
# ---------------------------------------
def pytest_configure(config : pytest.Config) -> None:
    '''
    Runs before all tests, needed to do global initialization. e.g. logging level setting
    '''
    _ = config

    logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)
    logging.getLogger("matplotlib.font_manager").setLevel(logging.WARNING)

    LogStore.set_level('rx_calibration:pizerodb', 10)

    _load_runs()
# ---------------------------------------
