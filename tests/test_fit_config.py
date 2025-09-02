'''
This module contains tests for the FitConfig class
'''

from omegaconf             import OmegaConf, DictConfig
from fitter.fit_config     import FitConfig
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:test_fit_config')
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

    cfg = FitConfig(fit_cfg = fit_cfg)
    cfg.replace(substring=org, value=new)

    assert cfg.fit_cfg == _get_config(name=new)
