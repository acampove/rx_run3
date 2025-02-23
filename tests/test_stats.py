'''
Module with testing functions for the Stats class
'''

from rx_data.stats import Stats

# ----------------------------------------
def test_simple():
    '''
    xxx
    '''
    obj = Stats(sample='Bd_JpsiX_ee_eq_JpsiInAcc')
    val = obj.get_entries(tree='MCDecayTree')
# ----------------------------------------
