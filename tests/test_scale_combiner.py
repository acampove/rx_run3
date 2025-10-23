'''
This module contains functions to test ScaleCombiner
'''
from rx_q2 import ScaleCombiner

def test_simple():
    '''
    Simplest test
    '''
    cmb = ScaleCombiner(version = 'v2')
    cmb.combine(name='parameters_ee.json', measurements=['rk_ee', 'rkst_ee'])
    cmb.combine(name='parameters_mm.json', measurements=['rk_mm', 'rkst_mm'])
