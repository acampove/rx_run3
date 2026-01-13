'''
This module contains tests for the FitConfig class
'''

import pytest
from pathlib           import Path
from omegaconf         import OmegaConf, DictConfig
from fitter.fit_config import FitConfig
from dmu               import LogStore
from dmu.generic       import utilities as gut
from rx_common         import Qsq, MVA

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
        'name' : { name : 1, 'a' : name },
        'model': {'components' : ['x', 'y']} 
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

    cfg = FitConfig(
        name    = 'test_replace', 
        group   = 'tests',
        mva_cmb = ['0.1', '0.2'],
        mva_prc = ['0.5'],
        q2bin   = Qsq.central,
        fit_cfg = fit_cfg)
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
        cfg = FitConfig(
            name    = name, 
            group   = 'tests',
            mva_cmb = ['0.1', '0.2'],
            mva_prc = ['0.5'],
            q2bin   = Qsq.central,
            fit_cfg = fit_cfg,
        )
        cfg.save(kind = 'test')

    assert cfg.fit_cfg == _get_config(name=name)
# ----------------------
def test_output(tmp_path : Path):
    '''
    Tests saving config 
    '''
    name    = 'test_save'
    block   = 3
    fit_cfg = _get_config(name = name)

    with gut.environment(mapping = {'ANADIR' : str(tmp_path)}):
        cfg = FitConfig(
            name    = name, 
            group   = 'tests',
            mva_cmb = ['0.1', '0.2'],
            mva_prc = ['0.5'],
            block   = block,
            q2bin   = Qsq.central,
            fit_cfg = fit_cfg,
        )
        cfg.save(kind = 'test')

        assert cfg.output_directory == tmp_path / f'fits/data/tests/010-020_050_b{block}'
# ----------------------
def test_str_to_wp():
    '''
    Test converter of strings to lists of floats
    '''
    assert FitConfig.str_to_wp(wp = '010_020'        , kind = MVA.cmb) == [0.1]
    assert FitConfig.str_to_wp(wp = '010_020'        , kind = MVA.prc) == [0.2]

    assert FitConfig.str_to_wp(wp = '010-030_020'    , kind = MVA.cmb) == [0.1, 0.3]
    assert FitConfig.str_to_wp(wp = '010_010-020'    , kind = MVA.prc) == [0.1, 0.2]

    assert FitConfig.str_to_wp(wp = '000-010_010-020', kind = MVA.cmb) == [0.0, 0.1]
    assert FitConfig.str_to_wp(wp = '000-010_010-020', kind = MVA.prc) == [0.1, 0.2]
# ----------------------

