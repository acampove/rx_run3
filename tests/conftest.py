'''
File needed by pytest
'''
import logging

import mplhep
import matplotlib.pyplot as plt

from _pytest.config import Config
# Needed to make sure zfit gets imported properly
# before any test runs
from dmu.stats.zfit import zfit

# ------------------------------
def pytest_configure(config : Config):
    '''
    This will run before any test by pytest
    '''
    _config = config

    logging.getLogger('PIL').setLevel(logging.WARNING)
    logging.getLogger('matplotlib').setLevel(logging.WARNING)

    plt.style.use(mplhep.style.LHCb2)
# ------------------------------
