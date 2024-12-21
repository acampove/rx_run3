'''
Module where config holder is tested
'''

from rx_kernel import ConfigHolder

# -----------------------------------
def test_simple():
    '''
    Simplest test of ConfigHolder
    '''
    ch = ConfigHolder()
    ch.Print()
# -----------------------------------
