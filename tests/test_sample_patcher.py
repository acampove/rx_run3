'''
Module with tests for SamplePatcher class
'''

import pytest
from rx_common.types import Trigger

from rx_data.sample_patcher import SamplePatcher
from rx_data.spec_maker import SpecMaker

_UNPATCHED_SAMPLES = [
    ('Bu_JpsiK_ee_eq_DPC', Trigger.rk_ee_os),
]
# ----------------------
@pytest.mark.parametrize('sample, trigger', _UNPATCHED_SAMPLES)
def test_unpatched(sample : str, trigger : Trigger) -> None:
    '''
    Tests that patching does not affect samples that are not
    meant to be patched
    '''
    spk  = SpecMaker(sample=sample, trigger=trigger)

    ptr  = SamplePatcher(sample = sample, spec = spk.spec)
    spec = ptr.get_patched_specification()

    assert spec == spk.spec
    assert len(ptr.redefinitions) == 0
