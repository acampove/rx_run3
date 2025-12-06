'''
File needed by pytest
'''
import os
os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
import logging
import numpy
import mplhep
import matplotlib.pyplot as plt

from _pytest.config import Config
# Needed to make sure zfit gets imported properly
# before any test runs
from dmu.stats.zfit        import zfit
from dmu.workflow.cache    import Cache as Wcache
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:conftest')
# ------------------------------
def pytest_configure(config : Config):
    '''
    This will run before any test by pytest
    '''
    _config = config
    numpy.random.seed(42)
    zfit.settings.set_seed(seed=42)

    logging.getLogger('PIL').setLevel(logging.WARNING)
    logging.getLogger('matplotlib').setLevel(logging.WARNING)
    logging.getLogger('git.cmd').setLevel(logging.WARNING)

    plt.style.use(mplhep.style.LHCb2)
    user = os.environ['USER']
    Wcache.set_cache_root(root=f'/tmp/{user}/tests/dmu/cache/cache_dir')
# ------------------------------
