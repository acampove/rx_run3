'''
This script tests FilteredStats class
'''

import pytest
from rx_data.filtered_stats import FilteredStats
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_data:test_filtered_stats')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:filtered_stats', 10)
# ----------------------
def test_simple():
    '''
    Simplest test of class
    '''
    fst = FilteredStats(analysis='rx', min_vers=8)
    df  = fst.get_df()
