'''
Module with tests for functions in generic/utilities.py
'''

from time import sleep
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
def test_dump_json():
    '''
    Tests dump_json
    '''
    gut.dump_json([1,2,3,4], '/tmp/tests/dmu/generic/list.json')
# -------------------------
def test_load_json():
    '''
    Tests load_json
    '''
    json_path = '/tmp/tests/dmu/generic/list.json'

    l_data_org = [1,2,3,4]
    gut.dump_json(l_data_org, json_path)

    l_data_lod = gut.load_json(json_path)

    assert l_data_org == l_data_lod
# -------------------------
