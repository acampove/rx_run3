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
    gut.dump_json([1,2,3,4], '/tmp/list.json')
# -------------------------
