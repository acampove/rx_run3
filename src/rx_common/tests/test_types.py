'''
Script with functions meant to test Enums
'''
import pytest

from rx_common import Component
from rx_common import Sample 
from rx_common import Brem 
from dmu       import LogStore

log=LogStore.add_logger('rx_common::test_types')
# -------------------------------------------
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
# -------------------------------------------
def test_brem():
    '''
    Test for Enum representing brem
    '''
    assert Brem.zero == 0
    assert Brem.one  == 1
    assert Brem.two  == 2
# -------------------------------------------
@pytest.mark.parametrize('sample', Sample)
def test_sample_properties(sample : Sample):
    '''
    Tests Sample enum properties
    '''
    assert isinstance(sample.name , str)
    assert isinstance(sample.latex, str)

    log.info(sample.name)
    log.info(sample.latex)
    log.info('')
# -------------------------------------------
def test_mc_samples():
    '''
    Tests that one can access list of MC samples
    '''
    mc_samples = Sample.get_mc_samples()
    assert isinstance(mc_samples, list)
    assert mc_samples

    for sample in mc_samples:
        log.info(sample)
# -------------------------------------------
