'''
File needed by pytest
'''
import logging

import numpy
import mplhep
import matplotlib.pyplot as plt
from _pytest.config import Config

from dmu.workflow.cache import Cache

# ------------------------------
def pytest_configure(config : Config):
    '''
    This will run before any test by pytest
    '''
    _config = config
    numpy.random.seed(42)

    logging.getLogger('PIL').setLevel(logging.WARNING)
    logging.getLogger('matplotlib').setLevel(logging.WARNING)

    Cache.set_cache_root(root='/tmp/tests/rx_efficiencies')

    plt.style.use(mplhep.style.LHCb2)
# ------------------------------
