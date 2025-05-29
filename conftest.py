import pytest
from dask.distributed import Client

@pytest.fixture(scope="session")  # One client for all tests
def dask_client():
    client = Client(processes=True)
    yield client
    client.close()
