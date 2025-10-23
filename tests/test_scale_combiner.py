'''
This module contains functions to test ScaleCombiner
'''
import pytest

from rx_q2                 import ScaleCombiner
from dmu.logging.log_store import LogStore

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_q2:scale_combiner', 10)
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    cmb = ScaleCombiner(version = 'v2')
    cmb.combine(name='parameters_ee.json', measurements=['rk_ee', 'rkst_ee'])
    cmb.combine(name='parameters_mm.json', measurements=['rk_mm', 'rkst_mm'])
# ----------------------
