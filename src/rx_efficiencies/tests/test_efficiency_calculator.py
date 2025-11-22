'''
Module with functions needed to test EfficiencyCalculator class
'''
import pytest
from dmu.workflow.cache                    import Cache
from dmu.logging.log_store                 import LogStore
from rx_selection                          import selection as sel
from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

_samples_rx = [
    'Bu_JpsiK_ee_eq_DPC',
    'Bu_Kee_eq_btosllball05_DPC']

_samples_nopid = [
    'Bu_JpsiK_ee_eq_DPC',
    'Bu_Kee_eq_btosllball05_DPC']

log = LogStore.add_logger('rx_efficiencies:test_efficiency_calculator')
#-------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before tests
    '''
    LogStore.set_level('rx_efficiencies:efficiency_calculator', 10)
#-------------------------------------------------
@pytest.mark.parametrize('sample',                _samples_rx)
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
def test_rx_efficiency_value(q2bin : str, sample : str):
    '''
    Tests retrieval of total efficiency (acceptance x reco x selection)
    for RX project samples
    '''
    with Cache.turn_off_cache(val=['EfficiencyCalculator']),\
         sel.custom_selection(d_sel={'bdt' : '(1)'}):
        obj      = EfficiencyCalculator(q2bin=q2bin, analysis='rx')
        eff, err = obj.get_efficiency(sample=sample)

    assert 0 <= eff < 1
    assert err > 0 or eff == 0
#-------------------------------------------------
@pytest.mark.parametrize('sample',             _samples_nopid)
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
def test_nopid_efficiency(q2bin : str, sample : str):
    '''
    Tests retrieval of total efficiency (acceptance x reco x selection)
    for RX project samples
    '''
    with Cache.turn_off_cache(val=['EfficiencyCalculator']),\
         sel.custom_selection(d_sel={'bdt' : '(1)'}):
        obj      = EfficiencyCalculator(
            q2bin   = q2bin, 
            sample  = sample,
            analysis= 'nopid', 
            trigger = 'Hlt2RD_BuToKpEE_MVA')
        eff, err = obj.get_efficiency(sample=sample)

    assert 0 <= eff < 1
    assert err > 0 or eff == 0
#-------------------------------------------------
