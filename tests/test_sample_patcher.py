'''
Module with tests for SamplePatcher class
'''

import pytest
from rx_common.types import Trigger

from rx_data.sample_patcher import SamplePatcher
from rx_data.spec_maker import SpecMaker

_SAMPLES = [
    ('Bu_JpsiK_ee_eq_DPC', Trigger.rk_ee_os),
]
# ----------------------
@pytest.mark.parametrize('sample, trigger', _SAMPLES)
def test_sample_patcher(sample : str, trigger : Trigger) -> None:
    '''
    Simplest test of sample patcher
    '''
    spk  = SpecMaker(sample=sample, trigger=trigger)

    ptr  = SamplePatcher(sample = sample, spec = spk.spec)
    spec = ptr.get_patched_specification()
    
