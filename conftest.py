'''
This module is used by pytest to _inject_ fixtures in the tests
'''

import pytest

from dask.distributed      import Client
from dmu.logging.log_store import LogStore

# ---------------------------------------
def pytest_configure(_config : pytest.Config) -> None:
    '''
    Runs before all tests, needed to do global initialization. e.g. logging level setting
    '''
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
    client = Client(processes=True)
    yield client
    client.close()
# ---------------------------------------
