'''
Module with tests for SpecMaker class
'''

import pytest

from rx_common.info     import LogStore
from rx_common.types    import Project, Trigger
from rx_data.spec_maker import SpecMaker, Specification

_SAMPLES=[
    ('Bu_JpsiK_ee_eq_DPC'            , Trigger.rk_ee_os),
    ('Bu_JpsiK_mm_eq_DPC'            , Trigger.rk_mm_os),
    ('Bd_Kstee_eq_btosllball05_DPC'  , Trigger.rkst_ee_os),
    ('Bd_Kstmumu_eq_btosllball05_DPC', Trigger.rkst_mm_os),
]

_PATCHING_SAMPLES = [
    ('Bs_JpsiX_mm_eq_JpsiInAcc', Trigger.rk_mm_os),
    ('Bu_JpsiX_mm_eq_JpsiInAcc', Trigger.rk_mm_os),
]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:spec_maker', 10)
# ------------------------------------------------------
@pytest.mark.parametrize('sample, trigger', _PATCHING_SAMPLES)
def test_patching(sample : str, trigger : Trigger) -> None:
    '''
    Return path to specification of combined sample
    '''
    mkr  = SpecMaker(sample=sample, trigger=trigger)
    path = mkr.get_spec_path(per_file=False)

    assert path.exists()
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
# ------------------------------------------------
@pytest.mark.parametrize('friend' , ['mva', 'brem_track_2'])
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('trigger', [Trigger.rk_ee_os, Trigger.rk_mm_os])
def test_exclude_friends(friend : str, sample : str, trigger : Trigger):
    '''
    Tests excluding friend trees through a context manager
    '''
    with SpecMaker.exclude_friends(names=[friend]):
        gtr = SpecMaker(sample=sample, trigger=trigger)
        path= gtr.get_spec_path(per_file=False)

    assert path.exists()

    json_string = path.read_text()
    spec = Specification.model_validate_json(json_string)

    assert friend not in spec.friends
# ------------------------------------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('trigger', [Trigger.rk_ee_os, Trigger.rk_mm_os])
def test_custom_friend(sample : str, trigger : Trigger):
    '''
    Tests getting data with a custom version for a given tree, either friend or main
    '''
    with SpecMaker.custom_friends(versions={'mva' : 'v8'}):
        gtr = SpecMaker(sample=sample, trigger=trigger)
        path= gtr.get_spec_path(per_file=False)

    assert path.exists()
# ------------------------------------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('trigger', [Trigger.rk_ee_os, Trigger.rk_mm_os])
def test_only_friends(sample : str, trigger : Trigger):
    '''
    Tests getting data with a custom version for a given tree, either friend or main
    '''
    s_friend = {'mva', 'hop'}
    with SpecMaker.only_friends(s_friend=s_friend):
        gtr = SpecMaker(sample=sample, trigger=trigger)
        path= gtr.get_spec_path(per_file=False)

    assert path.exists()
# ------------------------------------------------
@pytest.mark.parametrize('project', [Project.rk, Project.rk_no_refit])
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('trigger', [Trigger.rk_ee_os, Trigger.rk_mm_os])
def test_project(sample : str, trigger : Trigger, project : Project):
    '''
    Tests getting data with a custom version for a given tree, either friend or main
    '''
    with SpecMaker.project(name=project):
        gtr = SpecMaker(sample=sample, trigger=trigger)
        path= gtr.get_spec_path(per_file=False)

    assert path.exists()
