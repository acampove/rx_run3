'''
File needed by pytest
'''
import logging
import numpy
import mplhep
import matplotlib.pyplot as plt

from _pytest.config import Config
from dmu.logging.log_store import LogStore

# ----------------------
def _set_logs() -> None:
    '''
    Used to silence some loggers and put others in debug
    '''
    LogStore.set_level('rx_selection:selection'           , 30)
    LogStore.set_level('rx_data:rdf_getter'               , 30)
    LogStore.set_level('dmu:ml:cv_predict'                , 30)

    logging.getLogger('PIL').setLevel(logging.WARNING)
    logging.getLogger('matplotlib').setLevel(logging.WARNING)
# ------------------------------
def pytest_configure(config : Config):
    '''
    This will run before any test by pytest
    '''
    _config = config
    numpy.random.seed(42)
    _set_logs()

    plt.style.use(mplhep.style.LHCb2)
# ------------------------------
