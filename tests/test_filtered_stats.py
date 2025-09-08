'''
This script tests FilteredStats class
'''

from rx_data.filtered_stats import FilteredStats
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_data:filtered_stats')
# ----------------------
def test_simple():
    '''
    Simplest test of class
    '''
    fst = FilteredStats(analysis='rx')
    df  = fst.get_df()
