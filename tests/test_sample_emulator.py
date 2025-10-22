'''
Module holding tests for SampleEmulator class
'''

import pytest
from rx_data.sample_emulator import SampleEmulator

@pytest.mark.parametrize('old_name, expected_name',
    [
       ('Bd_JpsiKst_ee_eq_DPC', 'Bs_JpsiKst_ee_eq_DPC'),
       ('Bd_JpsiKst_mm_eq_DPC', 'Bs_JpsiKst_mm_eq_DPC'),
    ])
def test_rename(
    old_name      : str, 
    expected_name : str):
    '''
    Check renaming of samples
    '''
    emu = SampleEmulator(sample=old_name)
    new_name = emu.get_sample_name()

    assert new_name == expected_name
