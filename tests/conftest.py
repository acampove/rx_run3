'''
File needed by pytest
'''
import logging

import numpy
import mplhep
import matplotlib.pyplot as plt
from _pytest.config import Config

# ------------------------------
def pytest_configure(config : Config):
    '''
    This will run before any test by pytest
    '''
    _config = config
    numpy.random.seed(42)

    logging.getLogger('PIL').setLevel(logging.WARNING)
    logging.getLogger('matplotlib').setLevel(logging.WARNING)

    plt.style.use(mplhep.style.LHCb2)
# ------------------------------
