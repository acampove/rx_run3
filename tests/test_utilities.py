'''
Module with unit tests for utilities functions
'''

import os
from importlib.resources import files

import pytest

from dmu.logging.log_store import LogStore
import post_ap.utilities as ut

#----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('post_ap:utilities', 10)

    config_path               = files('post_ap_data').joinpath('tests/post_ap.yaml')
    os.environ['CONFIG_PATH'] = str(config_path)
#----------------------------------------
def test_simple():
    '''
    Test that it can read grid and local config
    '''
    d_cfg = ut.load_config()

    assert isinstance(d_cfg, dict)
#----------------------------------------
