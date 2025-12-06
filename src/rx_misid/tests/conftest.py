'''
Module storing data classes needed by tests
Also pytest functions intended to be run after tests
'''
import mplhep
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut

# -----------------------------------
def pytest_sessionstart():
    '''
    This will run before any test
    '''
    plt.style.use(mplhep.style.LHCb2)
    _set_logs()

    gut.TIMER_ON = True
# -----------------------------------
def _set_logs():
    LogStore.set_level('dmu:workflow:cache'   , 10)
    LogStore.set_level('rx_data:rdf_getter'   , 30)
    LogStore.set_level('rx_data:path_splitter', 30)
    LogStore.set_level('dmu:workflow:cache'   , 30)
    LogStore.set_level('rx_selection:truth_matching', 30)
    LogStore.set_level('rx_selection:selection'     , 30)
# -----------------------------------
