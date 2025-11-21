'''
Script with functions meant to test Enums
'''

from rx_common import Component
from rx_common import Brem 

def test_component():
    '''
    Test Enum representing fit component 
    '''
    assert Component.data          == 'data'
    assert Component.jpsi          == 'jpsi'
    assert Component.psi2          == 'psi2'
    assert Component.ccbar         == 'ccbar'
    assert Component.cabibbo       == 'cabibbo'
    assert Component.lbjpsipk      == 'lbjpsipk'
    assert Component.bsjpsiphi     == 'bsjpsiphi'
    assert Component.bsjpsikst     == 'bsjpsikst'
    assert Component.bdjpsikst_swp == 'bdjpsikst_swp'

def test_brem():
    '''
    Test for Enum representing brem
    '''
    assert Brem.zero == 0
    assert Brem.one  == 1
    assert Brem.two  == 2
