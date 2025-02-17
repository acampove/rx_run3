'''
Script with functions needed to test functions in selection.py
'''
from dmu.logging.log_store  import LogStore
from rx_selection import selection as sel

log=LogStore.add_logger('rx_selection:test_selection')
# --------------------------
def test_simple():
    '''
    Simplest test
    '''
    LogStore.set_level('rx_selection:selection', 10)
    d_sel = sel.selection(project='RK', analysis='EE', q2bin='central', process='DATA')
    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
# --------------------------
