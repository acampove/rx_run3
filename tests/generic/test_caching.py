'''
This file is meant to hold tests for the caching
utilities in generic/caching.py
'''
import math
from typing import Any

import pytest

from dmu.logging.log_store import LogStore
from dmu.generic           import caching
from dmu.generic           import utilities as gut 

log=LogStore.add_logger('dmu:test_caching')
# ----------------------
@pytest.mark.parametrize('obj', [
    1,
    1.3,
    [1,2],
    {'a' : 1}])
def test_cache(obj : Any):
    '''
    Tests dumping and loading data in files with names as hashes
    '''
    caching.cache_data(obj, hash_obj=obj)
    ret = caching.load_cached(hash_obj=obj)

    if isinstance(obj, float):
        math.isclose(obj, ret, rel_tol=1e-5)
    else:
        assert ret == obj
# -------------------------
def test_cache_not_found():
    '''
    Checks that what will happen when a cached file is not found happens
    '''
    with pytest.raises(FileNotFoundError):
        ret = gut.load_cached(hash_obj=['something that will never be cached'])

    ret = gut.load_cached(hash_obj=['something that will never be cached'], on_fail=-999)

    assert ret == -999
# -------------------------
