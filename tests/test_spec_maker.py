'''
Module with tests for SpecMaker class
'''

import pytest

from rx_common.info import LogStore
from rx_common.types    import Trigger
from rx_data.spec_maker import SpecMaker

_SAMPLES=[
    ('Bu_JpsiK_ee_eq_DPC'            , 'Hlt2RD_BuToKpEE_MVA'),
    ('Bu_JpsiK_mm_eq_DPC'            , 'Hlt2RD_BuToKpMuMu_MVA'),
    ('Bd_Kstee_eq_btosllball05_DPC'  , 'Hlt2RD_B0ToKpPimEE_MVA'),
    ('Bd_Kstmumu_eq_btosllball05_DPC', 'Hlt2RD_B0ToKpPimMuMu_MVA'),
]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:spec_maker', 10)
# ------------------------------------------------------
@pytest.mark.parametrize('sample, trigger', _SAMPLES)
def test_combined(sample : str, trigger : Trigger) -> None:
    '''
    Return path to specification of combined sample
    '''
    mkr  = SpecMaker(sample=sample, trigger=trigger)
    path = mkr.get_spec_path(per_file=False)

    assert path.exists()
# ------------------------------------------------------
@pytest.mark.parametrize('sample, trigger', _SAMPLES)
def test_per_file(sample : str, trigger : Trigger) -> None:
    '''
    Return path to specification of combined sample
    '''
    mkr  = SpecMaker(sample=sample, trigger=trigger)
    paths= mkr.get_spec_path(per_file=True)

    for root_path, config_path in paths.items():
        assert root_path.exists()
        assert config_path.exists()

