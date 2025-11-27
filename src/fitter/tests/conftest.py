'''
This file is needed by pytest
'''
import os
os.environ["CUDA_VISIBLE_DEVICES"] = '-1'
from pathlib import Path

import mplhep
import matplotlib.pyplot as plt
import pytest

from rx_selection                import selection  as sel
from _pytest.config              import Config

from dmu.stats.zfit              import zfit
from dmu.workflow.cache          import Cache
from dmu.logging.log_store       import LogStore
from zfit.interface              import ZfitParameter as zpar
from zfit.interface              import ZfitSpace     as zobs

executed_tests = set()
log = LogStore.add_logger('rx_efficiencies:conftest')

# --------------------------------------------------------------
def pytest_addoption(parser):
    parser.addoption('--ntoys', type=int, default=-1, help='Override number of toys, by default will do what is in the config')

@pytest.fixture
def ntoys(request):
    return request.config.getoption('--ntoys')

@pytest.fixture(scope='session')
def test_dir() -> Path:
    user     = os.environ.get('USER', 'unknown')
    dir_path = Path(f'/tmp/{user}/tests/fitter')
    dir_path.mkdir(parents=True, exist_ok=True)

    return dir_path
# --------------------------------------------------------------
def _set_logs() -> None:
    LogStore.set_level('fitter:data_model'          , 10)
    LogStore.set_level('fitter:sim_fitter'          , 10)
    LogStore.set_level('fitter:data_fitter'         , 10)
    LogStore.set_level('fitter:base_fitter'         , 10)
    LogStore.set_level('fitter:data_preprocessor'   , 10)
    LogStore.set_level('fitter:prec'                , 10)
    LogStore.set_level('fitter:prec_scales'         , 10)
    LogStore.set_level('rx_fitter:constraint_reader', 10)

    # Silence what is below

    LogStore.set_level('rx_efficiencies:efficiency_calculator', 30)
    LogStore.set_level('rx_selection:selection'               , 30)
    LogStore.set_level('rx_selection:truth_matching'          , 30)
    LogStore.set_level('rx_data:path_splitter'                , 30)
    LogStore.set_level('rx_data:rdf_getter'                   , 30)
    LogStore.set_level('dmu:workflow:cache'                   , 30)
    LogStore.set_level('dmu:stats:model_factory'              , 30)
    LogStore.set_level('dmu:zfit_plotter'                     , 30)

    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
    os.environ['GRPC_VERBOSITY'] = 'ERROR'
# ----------------------------------------
def pytest_configure(config : Config):
    '''
    This function is called before any test run
    '''
    _config = config

    # Use this as the root directory where everything gets
    # cached

    user      = os.environ['USER']
    cache_dir = f'/tmp/{user}/tests/fitter'
    Cache.set_cache_root(root=cache_dir)

    _set_logs()

    plt.style.use(mplhep.style.LHCb2)
# ----------------------------------------
@pytest.fixture
def skip_mass_cut():
    '''
    This is a fixture meant to be passed as an argumen to tests
    It will ensure that the test is ran with data without the mass cut
    '''
    with sel.custom_selection(d_sel = {'mass' : '(1)'}):
        yield
# ----------------------------------------
@pytest.fixture
def pytest_runtest_logreport(report):
    '''
    Will collect the names (?) of the tests that were ran and passed
    in the executed_tests set
    '''
    if report.when == "call" and report.passed:
        executed_tests.add(report.nodeid)
# -----------------------------------
