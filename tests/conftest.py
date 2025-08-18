'''
This module is needed to control how pytest will run tests
'''
import os

import mplhep
import pytest
import matplotlib.pyplot as plt
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

    LogStore.set_level('rx_data:mass_calculator'          , 10)
    LogStore.set_level('rx_data:mva_calculator'           , 10)
    LogStore.set_level('rx_data:test_mva_calculator'      , 10)
    LogStore.set_level('rx_data:rdf_getter12'             , 10)
    LogStore.set_level('rx_data:rdf_getter'               , 10)
    LogStore.set_level('rx_selection:selection'           , 30)
    LogStore.set_level('dmu:ml:cv_predict'                , 30)
# -----------------------------------------------
@pytest.fixture(autouse=True)
def rdf_getter_configuration():
    '''
    This will configure RDFGetter for tests
    '''
    plt.style.use(mplhep.style.LHCb2)

    with RDFGetter.max_entries(value=1000):
        yield

    gut.TIMER_ON = True
# -----------------------------------------------
@pytest.fixture(scope='session')
def out_dir() -> str:
    '''
    This is a fixture meant to be passed as an argument
    to the tests to make the path to the output directory
    available to them
    '''
    user = os.environ['USER']
    path = f'/tmp/{user}/tests/rx_data'
    os.makedirs(path, exist_ok=True)

    return path
# ------------------------------
