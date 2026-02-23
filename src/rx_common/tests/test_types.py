'''
Script with functions meant to test Enums
'''
import pytest

from dmu       import LogStore

from rx_common import Component
from rx_common import Brem 
from rx_common import Channel 
from rx_common import Trigger
from rx_common import Project 

log=LogStore.add_logger('rx_common::test_types')
# -------------------------------------------
def test_component():
    '''
    Test Enum representing fit component 
    '''
    assert Component.data_24         == 'DATA_24*'
    assert Component.ccbar           == 'ccbar'

    assert Component.bpkpjpsiee      == 'Bu_JpsiK_ee_eq_DPC'
    assert Component.bpkppsi2ee      == 'Bu_psi2SK_ee_eq_DPC'
    assert Component.bppijpsiee      == 'Bu_JpsiPi_ee_eq_DPC'
    # ------
    assert Component.bsphijpsimm     == 'Bs_Jpsiphi_mm_eq_CPV_update2012_DPC'
    assert Component.bsphijpsiee     == 'Bs_Jpsiphi_ee_eq_CPV_update2012_DPC'
    # ------
    assert Component.lbpkjpsimm      == 'Lb_JpsipK_mm_eq_phsp_DPC'
    assert Component.lbpkjpsiee      == 'Lb_JpsipK_ee_eq_phsp_DPC'
    # ------
    assert Component.bskstjpsimm     == 'Bs_JpsiKst_mm_eq_DPC'
    assert Component.bskstjpsiee     == 'Bs_JpsiKst_ee_eq_DPC'

    assert Component.bdkstjpsimm_swp == 'Bd_JpsiKst_mm_had_swp'
    assert Component.bdkstjpsiee_swp == 'Bd_JpsiKst_ee_had_swp'
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
    # There are no subdecays for inclusive samples
    if 'JpsiX' in sample:
        return

    if sample in ['undefined']:
        return

    assert len(sample.subdecays) >= 1
    assert isinstance(sample.subdecays, list)
