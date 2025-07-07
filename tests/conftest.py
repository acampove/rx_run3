'''
This file is needed by pytest
'''

import pytest

from dmu.workflow.cache import Cache

@pytest.fixture(scope='session', autouse=True)
def setup_session():
    '''
    This function is called before any test run
    '''
    # Use this as the root directory where everything gets
    # cached
    Cache.set_cache_root(root='/tmp/tests/fitter')
