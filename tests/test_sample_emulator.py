'''
Module holding tests for SampleEmulator class
'''

import pytest
from rx_data.sample_emulator import SampleEmulator

@pytest.mark.parametrize('old_name, new_name',
    [
       ('Bs_JpsiKst_ee_eq_DPC', 'Bd_JpsiKst_ee_eq_DPC'),
       ('Bs_JpsiKst_mm_eq_DPC', 'Bd_JpsiKst_mm_eq_DPC'),
    ])
def test_rename(
    old_name : str, 
    new_name : str):
    '''
    Check renaming of samples

    old_name: Sample requested
    new_name: Expected sample to use
    '''
    emu = SampleEmulator(sample=old_name)
    val = emu.get_sample_name()

    assert new_name == val 
