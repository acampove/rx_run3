'''
This module is used by pytest to _inject_ fixtures in the tests
'''
import os
import logging
import pytest

from dask.distributed      import Client
from dmu.logging.log_store import LogStore

# ---------------------------------------
def pytest_configure(config : pytest.Config) -> None:
    '''
    Runs before all tests, needed to do global initialization. e.g. logging level setting
    '''
    _ = config

    os.environ['ANADIR'] = '/tmp/tests/ecal_calibration'

    logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)
    logging.getLogger("matplotlib.font_manager").setLevel(logging.WARNING)

    LogStore.set_level('ecal_calibration:regressor'   , 10)
    LogStore.set_level('ecal_calibration:preprocessor', 10)
# ---------------------------------------
@pytest.fixture(scope='session')
def _dask_client():
    '''
    Create a dask client to:

    - Turn off multithreading. Due to lack of thread safety in vector
    - Turn on multiprocessing
    '''
    client = Client(n_workers=1, threads_per_worker=1, processes=True)

    yield client
    client.close()
# ---------------------------------------
