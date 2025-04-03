'''
Module with functions needed to test EfficiencyCalculator class
'''
import pytest
from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

#-------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str):
    '''
    Tests calculation of total efficiency (acceptance x reco x selection)
    for multiple MC samples
    '''
    obj         = EfficiencyCalculator(proc=None, year='2024', q2bin=q2bin)
    obj.out_dir = '/tmp/tests/rx_efficiencies/efficiency_calculator/simple/{q2bin}'
    df          = obj.get_stats()

    print(df)
#-------------------------------------------------
