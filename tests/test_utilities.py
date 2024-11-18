'''
Module with unit tests for utilities functions
'''

import pytest

from dmu.logging.log_store import LogStore
import post_ap.utilities as ut

#----------------------------------------
class Data:
    '''
    Class holding shared attributes
    '''

    l_arg_kind = [
            ('dt_2024_turbo_comp', 'toml', True),
            ('dt_2024_turbo_comp', 'toml', False),
            ('hlt_cmp'           , 'yaml', True)]
#----------------------------------------
@pytest.fixture
def _initialize():
    LogStore.set_level('post_ap:utilities', 10)
#----------------------------------------
@pytest.mark.parametrize('is_local', [True, False])
def test_simple(is_local : bool):
    '''
    Test that it can read grid and local config
    '''
    ut.local_config=is_local

    d_cfg = ut.load_config('dt_2024_turbo_comp')

    assert isinstance(d_cfg, dict)
#----------------------------------------
@pytest.mark.parametrize('name, kind, is_local', Data.l_arg_kind)
def test_kind(name : str , kind :str, is_local : bool):
    '''
    Test that it can load yaml and toml locally and from grid
    '''
    ut.local_config=is_local

    d_cfg = ut.load_config(name, kind)
    assert isinstance(d_cfg, dict)
#----------------------------------------
