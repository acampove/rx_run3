'''
Module with tests for functions in generic/utilities.py
'''
from time import sleep

import pytest
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
