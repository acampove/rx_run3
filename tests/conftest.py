'''
This file is needed by pytest
'''
import os
import mplhep
import matplotlib.pyplot as plt

from _pytest.config import Config

from dmu.workflow.cache    import Cache
from dmu.logging.log_store import LogStore

# ----------------------------------------
def _set_logs() -> None:
    LogStore.set_level('fitter:data_model'          , 10)
    LogStore.set_level('fitter:sim_fitter'          , 10)
    LogStore.set_level('fitter:data_preprocessor'   , 10)
    LogStore.set_level('fitter:prec'                , 10)

    # Silence what is below
    LogStore.set_level('rx_selection:selection'     , 30)
    LogStore.set_level('rx_selection:truth_matching', 30)
    LogStore.set_level('rx_data:path_splitter'      , 30)
    LogStore.set_level('rx_data:rdf_getter'         , 30)
    LogStore.set_level('dmu:workflow:cache'         , 30)
    LogStore.set_level('dmu:stats:model_factory'    , 30)
    LogStore.set_level('dmu:zfit_plotter'           , 30)

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
    Cache.set_cache_root(root='/tmp/tests/fitter')
    _set_logs()

    plt.style.use(mplhep.style.LHCb2)
# ----------------------------------------
