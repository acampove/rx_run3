'''
Module with functions needed to test EfficiencyCalculator class
'''
import pytest
from dmu.logging.log_store                 import LogStore
from rx_selection                          import selection as sel
from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

log = LogStore.add_logger('rx_efficiencies:test_efficiency_calculator')
#-------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_efficiencies:efficiency_calculator', 10)
#-------------------------------------------------
@pytest.mark.parametrize('sample', ['Bu_JpsiK_ee_eq_DPC'])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
def test_efficiency(q2bin : str, sample : str):
    '''
    Tests retrieval of total efficiency (acceptance x reco x selection)
    for given sample
    '''
    with sel.custom_selection(d_sel={'bdt' : '1'}):
        obj         = EfficiencyCalculator(q2bin=q2bin)
        eff         = obj.get_efficiency(sample=sample)

    assert 0 <= eff < 1
#-------------------------------------------------
