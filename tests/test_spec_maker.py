'''
Module with tests for SpecMaker class
'''

import pytest

from rx_common.types    import Trigger
from rx_data.spec_maker import SpecMaker

_SAMPLES=[
    ('Bu_JpsiK_ee_eq_DPC', 'Hlt2RD_BuToKpEE_MVA'),
]
# ------------------------------------------------------
@pytest.mark.parametrize('sample, trigger', _SAMPLES)
def test_simple(sample : str, trigger : Trigger) -> None:
    '''
    Simplest test of SpecMaker
    '''
    mkr  = SpecMaker(sample=sample, trigger=trigger)
    path = mkr.get_path()

    assert path.exists()
