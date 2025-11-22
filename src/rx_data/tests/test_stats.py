'''
Module with testing functions for the Stats class
'''

import pytest
from dmu.logging.log_store import LogStore
from rx_data.stats         import Stats

log=LogStore.add_logger('rx_data:test_stats')
# ----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:stats', 10)
    Stats.d_sample = {
            'main' : '/home/acampove/external_ssd/Data/samples/main.yaml',
            }
# ----------------------------------------
def test_mcdt():
    '''
    Tests MCDecayTree statistics retrieval
    '''
    obj = Stats(sample='Bu_JpsiK_ee_eq_DPC', trigger='Hlt2RD_BuToKpEE_MVA')
    val = obj.get_entries(tree='MCDecayTree')

    assert val > 0
# ----------------------------------------
