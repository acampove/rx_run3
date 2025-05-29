'''
This module is used by pytest to _inject_ fixtures in the tests
'''

import pytest

from dask.distributed import Client

# ---------------------------------------
@pytest.fixture(scope='session')
def dask_client():
    '''
    Create a dask client to:

    - Turn off multithreading. Due to lack of thread safety in vector
    - Turn on multiprocessing
    '''
    client = Client(processes=True)
    yield client
    client.close()
# ---------------------------------------
