'''
Script with functions meant to test Enums
'''
import ROOT
import pytest

from rx_common import Correction
from rx_common import Component
from rx_common import Sample 
from rx_common import Brem 
from rx_common import Channel 
from dmu       import LogStore

from rx_common import Trigger
from rx_common import Project 

log=LogStore.add_logger('rx_common::test_types')
# -------------------------------------------
@pytest.mark.parametrize('corr', Correction)
def test_correction(corr : Correction):
    '''
    Tests Correction type
    '''
    assert isinstance(corr.latex, str)
    assert corr.kind in {'reso', 'scale'} 
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
    assert Brem.zero    == '000'
    assert Brem.one     == '001'
    assert Brem.two     == '002'
    assert Brem.one_two == '012'

    assert Brem.one + Brem.two == Brem.one_two
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
@pytest.mark.parametrize('sample', Sample)
def test_channel(sample : Sample):
    '''
    Tests channel property of samples
    '''
    if   sample.name.endswith('ee'):
        expected = Channel.ee
    elif sample.name.endswith('mm'):
        expected = Channel.mm
    else:
        expected = None

    if expected is not None:
        assert sample.channel == expected
        return

    with pytest.raises(ValueError):
        sample.channel
# -------------------------------------------
@pytest.mark.parametrize('trigger', Trigger)
def test_trigger(trigger : Trigger):
    '''
    Test trigger enum
    '''

    if   trigger.name.startswith('rk_'):
        expected = Project.rk
    elif trigger.name.startswith('rkst_'):
        expected = Project.rkst
    else:
        expected = None

    if expected is not None:
        assert trigger.project == expected
        return

    with pytest.raises(ValueError):
        trigger.project
# -------------------------------------------
@pytest.mark.parametrize('sample', Sample.get_mc_samples())
def test_subdecays(sample : Sample):
    '''
    Tests access to subdecays from sample
    '''
    samples = [
        Sample.undefined, 
        Sample.ccbar, 
        # --------------
        Sample.bpjpsixee,
        Sample.bdjpsixee,
        Sample.bsjpsixee,
        # --------------
        Sample.bpjpsixmm,
        Sample.bdjpsixmm,
        Sample.bsjpsixmm,
        # --------------
        Sample.bpkkk,
        Sample.bpkpik,
        Sample.bpkpipi,
    ]

    if sample in samples:
        return

    assert isinstance(sample.subdecays, list)
