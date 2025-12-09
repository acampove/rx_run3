'''
This module contains tests for the FitConfig class
'''

import pytest
from pathlib           import Path
from omegaconf         import OmegaConf, DictConfig
from fitter.fit_config import FitConfig
from dmu               import LogStore
from dmu.generic       import utilities as gut

log=LogStore.add_logger('fitter:test_fit_config')
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('fitter:fit_config', 10)
# ----------------------
def _get_config(name : str) -> DictConfig:
    fit_cfg = {
        name   : [1,2,3],
        'name' : { name : 1, 'a' : name }
    }

    return OmegaConf.create(fit_cfg)
# ----------------------
def test_replace():
    '''
    Tests replacing substring in the configuration
    '''
    org     = 'brem_xxx'
    new     = '7634244'
    fit_cfg = _get_config(name = org)

    cfg = FitConfig(name = 'test_replace', fit_cfg = fit_cfg)
    cfg.replace(substring=org, value=new)

    assert cfg.fit_cfg == _get_config(name=new)
# ----------------------
def test_save(tmp_path : Path):
    '''
    Tests saving config 
    '''
    name    = 'test_save'
    fit_cfg = _get_config(name = name)

    with gut.environment(mapping = {'ANADIR' : str(tmp_path)}):
        cfg = FitConfig(name = name, fit_cfg = fit_cfg)
        cfg.save(kind = 'test')

    assert cfg.fit_cfg == _get_config(name=name)
