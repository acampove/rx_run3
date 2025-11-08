'''
Module with tests for SamplePatcher class
'''

import pytest
from dmu.logging.log_store  import LogStore
from rx_common.types        import Trigger
from rx_data.sample_patcher import SamplePatcher
from rx_data.spec_maker     import SpecMaker

_UNPATCHED_SAMPLES = [
    ('Bu_JpsiK_ee_eq_DPC'            , Trigger.rk_ee_os),
    ('Bu_JpsiK_mm_eq_DPC'            , Trigger.rk_mm_os),
    ('Bd_Kstee_eq_btosllball05_DPC'  , Trigger.rkst_ee_os),
    ('Bd_Kstmumu_eq_btosllball05_DPC', Trigger.rkst_mm_os),
]
_PATCHED_SAMPLES = [
    ('Bs_JpsiX_mm_eq_JpsiInAcc', Trigger.rk_mm_os),
    ('Bu_JpsiX_mm_eq_JpsiInAcc', Trigger.rk_mm_os),
]

log=LogStore.add_logger('rx_data:test_sample_patching')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:sample_patcher', 10)
    LogStore.set_level('rx_data:spec_maker'    , 30)
    LogStore.set_level('rx_data:path_splitter' , 30)
    LogStore.set_level('rx_data:rdf_getter'    , 30)
# ----------------------
@pytest.mark.parametrize('sample, trigger', _UNPATCHED_SAMPLES)
def test_unpatched(sample : str, trigger : Trigger) -> None:
    '''
    Tests that patching does not affect samples that are not
    meant to be patched
    '''
    spk  = SpecMaker(sample=sample, trigger=trigger)
    spec_old = spk.spec

    ptr  = SamplePatcher(sample = sample, spec = spec_old)
    spec_new = ptr.get_patched_specification()

    assert spec_old == spec_new
    assert len(ptr.redefinitions) == 0
# ----------------------
@pytest.mark.parametrize('sample, trigger', _PATCHED_SAMPLES)
def test_patched(sample : str, trigger : Trigger) -> None:
    '''
    Tests that patching
    '''
    spk = SpecMaker(sample=sample, trigger=trigger, skip_patch=True)
    spec_old = spk.spec

    ptr = SamplePatcher(sample = sample, spec = spec_old)
    spec_new = ptr.get_patched_specification()

    assert spec_old != spec_new
    assert len(ptr.redefinitions) != 0
# ----------------------
