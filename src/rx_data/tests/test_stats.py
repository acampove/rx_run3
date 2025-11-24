'''
Module with testing functions for the Stats class
'''

import pytest
from dmu        import LogStore
from rx_common  import Trigger
from rx_data    import Stats

log=LogStore.add_logger('rx_data:test_stats')
# ----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before the tests
    '''
    LogStore.set_level('rx_data:stats', 10)
# ----------------------------------------
def test_mcdt():
    '''
    Tests MCDecayTree statistics retrieval
    '''
    obj = Stats(sample='Bu_JpsiK_ee_eq_DPC', trigger=Trigger.rk_ee_os)
    val = obj.get_entries(tree='MCDecayTree')

    assert val > 0
# ----------------------------------------
