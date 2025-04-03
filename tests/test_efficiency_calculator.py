'''
Module with functions needed to test EfficiencyCalculator class
'''

from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

#-------------------------------------------------
def test_simple():
    '''
    Tests calculation of total efficiency (acceptance x reco x selection)
    for multiple MC samples
    '''
    obj         = EfficiencyCalculator(proc=None, year='2024')
    obj.out_dir = '/tmp/tests/rx_efficiencies/efficiency_calculator/simple'
    df          = obj.get_stats()

    print(df)
#-------------------------------------------------
