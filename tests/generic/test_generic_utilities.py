'''
Module with tests for functions in generic/utilities.py
'''
import math
from typing import Any
from time   import sleep

import pytest
from omegaconf import DictConfig
import dmu.generic.utilities as gut

# -------------------------
def test_timeit():
    '''
    Will test timer
    '''
    gut.TIMER_ON=True
    @gut.timeit
    def fun():
        sleep(3)

    fun()
# -------------------------
@pytest.mark.parametrize('ext', ['json', 'yaml'])
def test_dump_json(ext : str):
    '''
    Tests dump_json
    '''
    gut.dump_json([1,2,3,4], f'/tmp/tests/dmu/generic/list.{ext}')
# -------------------------
@pytest.mark.parametrize('ext', ['json', 'yaml'])
def test_load_json(ext : str):
    '''
    Tests load_json
    '''
    json_path = f'/tmp/tests/dmu/generic/list.{ext}'

    l_data_org = [1,2,3,4]
    gut.dump_json(l_data_org, json_path)

    l_data_lod = gut.load_json(json_path)

    assert l_data_org == l_data_lod
# -------------------------
def test_dump_pickle():
    '''
    Tests dump_json and loading it
    '''
    path = '/tmp/tests/dmu/generic/list.pkl'
    data = [1,2,3,4]
    gut.dump_pickle(data=data, path=path)

    loaded = gut.load_pickle(path=path)

    assert data == loaded
# -------------------------
def test_silent_import():
    '''
    Tests decorator in charge of suppressing
    messages from imported modules
    '''
    with gut.silent_import():
        import tensorflow
# -------------------------
@pytest.mark.parametrize('ext', ['yaml', 'json'])
def test_load_data(ext : str):
    '''
    Tests loading file from data package
    '''
    expected = {
            "key" : [
                "value1",
                "value2",
                "value3"]
            }

    data = gut.load_data(
            package='dmu_data',
            fpath  =f'tests/config.{ext}')

    assert data == expected
# -------------------------
@pytest.mark.parametrize('ext', ['yaml', 'json'])
def test_load_conf(ext : str):
    '''
    Tests loading configuration 
    '''
    cfg = gut.load_conf(
            package='dmu_data',
            fpath  =f'tests/config.{ext}')

    assert isinstance(cfg, DictConfig)

    assert cfg.key == ['value1', 'value2', 'value3']
# -------------------------
@pytest.mark.parametrize('obj', [
    1,
    1.3,
    [1,2],
    {'a' : 1}])
def test_cache(obj : Any):
    '''
    Tests dumping and loading data in files with names as hashes
    '''
    gut.cache_data(obj, hash_obj=obj)
    ret = gut.load_cached(hash_obj=obj)

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
