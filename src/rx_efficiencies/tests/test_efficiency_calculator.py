'''
Module with functions needed to test EfficiencyCalculator class
'''
import pytest
from dmu.workflow.cache                    import Cache
from dmu.logging.log_store                 import LogStore
from rx_common.types                       import Trigger
from rx_data.rdf_getter                    import RDFGetter
from rx_selection                          import selection as sel
from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

_SAMPLES_RX    = [
    'Bu_JpsiK_ee_eq_DPC',
    'Bu_Kee_eq_btosllball05_DPC']

_SAMPLES_NOPID = [
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
    LogStore.set_level('rx_data:rdf_getter'    , 10)
    LogStore.set_level('rx_data:spec_maker'    , 10)
    LogStore.set_level('rx_data:sample_patcher', 10)

    with RDFGetter.max_entries(value = 1000):
        yield
#-------------------------------------------------
@pytest.mark.parametrize('sample',                _SAMPLES_RX)
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
def test_rx_efficiency_value(q2bin : str, sample : str):
    '''
    Tests retrieval of total efficiency (acceptance x reco x selection)
    for RX project samples
    '''
    with Cache.turn_off_cache(val=['EfficiencyCalculator']),\
         sel.custom_selection(d_sel={'bdt' : '(1)'}):
        obj      = EfficiencyCalculator(q2bin=q2bin)
        eff, err = obj.get_efficiency(sample=sample)

    assert 0 <= eff < 1
    assert err > 0 or eff == 0
#-------------------------------------------------
@pytest.mark.parametrize('sample',             _SAMPLES_NOPID)
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
            trigger = Trigger.rk_ee_os)
        eff, err = obj.get_efficiency(sample=sample)

    assert 0 <= eff < 1
    assert err > 0 or eff == 0
#-------------------------------------------------
