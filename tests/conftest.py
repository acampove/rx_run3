'''
This file is needed by pytest
'''

from _pytest.config import Config

from dmu.workflow.cache    import Cache
from dmu.logging.log_store import LogStore

# ----------------------------------------
def _set_logs() -> None:
    LogStore.set_level('dmu:workflow:cache', 10)
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
