'''
Module with functions needed to test EfficiencyCalculator class
'''
import pytest
from dmu.logging.log_store                 import LogStore
from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

log = LogStore.add_logger('rx_efficiencies:test_efficiency_calculator')
#-------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_efficiencies:efficiency_calculator', 10)
#-------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_stats(q2bin : str):
    '''
    Tests calculation of total efficiency (acceptance x reco x selection)
    for multiple MC samples
    '''
    obj         = EfficiencyCalculator(q2bin=q2bin)
    obj.out_dir = '/tmp/tests/rx_efficiencies/efficiency_calculator/stats'
    df          = obj.get_stats()

    assert len(df) > 0
#-------------------------------------------------
