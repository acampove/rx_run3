'''
This script tests FilteredStats class
'''

import pytest
import pandas as pnd
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
    pnd.set_option('display.max_columns' , None)
    pnd.set_option('display.max_colwidth', None)
    pnd.set_option('display.width'       ,    0)
# ----------------------
def test_simple():
    '''
    Simplest test of class
    '''
    fst = FilteredStats(analysis='rx', versions=[7, 10])
    df  = fst.get_df()

    print(df)
# ----------------------
def test_all_lfns():
    '''
    Search for information for all the LFNs in the JSON files
    '''
    fst = FilteredStats(analysis='rx', versions=[7, 10])
    df  = fst.get_df()

    print(df)
