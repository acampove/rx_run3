'''
Module with tests for functions in generic/utilities.py
'''

from time import sleep
import dmu.generic.utilities as gut

# -------------------------
def test_timeit():
    gut.TIMER_ON=True
    @gut.timeit
    def fun():
        sleep(3)

    fun()
# -------------------------
