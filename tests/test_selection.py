'''
Script with functions needed to test functions in selection.py
'''
import pytest

from dmu.logging.log_store  import LogStore
from rx_selection import selection as sel

log=LogStore.add_logger('rx_selection:test_selection')
# --------------------------
@pytest.mark.parametrize('analysis', ['EE', 'MM'])
@pytest.mark.parametrize('q2bin'   , ['low', 'central', 'high'])
def test_simple(analysis : str, q2bin : str):
    '''
    Simplest test
    '''
    LogStore.set_level('rx_selection:selection', 10)
    d_sel = sel.selection(project='RK', analysis=analysis, q2bin=q2bin, process='DATA')
    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
# --------------------------
