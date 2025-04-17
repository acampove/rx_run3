'''
Script meant to test MisIDCalculator class
'''
from importlib.resources import files

import yaml
from dmu.logging.log_store     import LogStore
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:test_misid_calculator')
# ---------------------------------
def _get_config() -> dict:
    config_path = files('rx_misid_data').joinpath('config.yaml')
    with open(config_path, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    return data
# ---------------------------------
def test_simple():
    '''
    Simplest example of misid calculator usage
    '''
    cfg = _get_config()
    obj =MisIDCalculator(cfg=cfg)
    data=obj.get_misid()
# ---------------------------------
