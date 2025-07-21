'''
This module is needed to control how pytest will run tests
'''

import pytest
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore
from _pytest.config        import Config
from rx_data.rdf_getter    import RDFGetter

# ------------------------------
def pytest_configure(config : Config):
    '''
    This will run before any test by pytest
    '''
    _config = config

    LogStore.set_level('rx_data:rdf_getter12', 10)
    LogStore.set_level('rx_data:rdf_getter'  , 10)
# -----------------------------------------------
@pytest.fixture(autouse=True)
def rdf_getter_configuration():
    '''
    This will configure RDFGetter for tests
    '''
    with RDFGetter.max_entries(value=1000):
        yield

    gut.TIMER_ON = True
# -----------------------------------------------
