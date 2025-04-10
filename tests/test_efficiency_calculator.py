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
def test_simple(q2bin : str):
    '''
    Tests calculation of total efficiency (acceptance x reco x selection)
    for multiple MC samples
    '''
    obj         = EfficiencyCalculator(q2bin=q2bin)
    obj.out_dir = '/tmp/tests/rx_efficiencies/efficiency_calculator/simple'
    df          = obj.get_stats()

    print(df)
#-------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_custom_selection(q2bin : str):
    '''
    Tests calculation of total efficiency where the selection has been overriden
    '''
    d_cut       = {'mva' : 'mva_cmb > 0.90 && mva_prc > 0.85'}

    obj         = EfficiencyCalculator(q2bin=q2bin, d_cut=d_cut)
    obj.out_dir = '/tmp/tests/rx_efficiencies/efficiency_calculator/custom_selection'
    df          = obj.get_stats()

    print(df)
#-------------------------------------------------
