'''
Module containing tests for SampleConfig class
'''
from omegaconf import DictConfig, OmegaConf

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ap_utilities:test_sample_config')
# ----------------------

def test_simple():
    '''
    Simplest test
    '''
    obj = SampleConfig(settings='2024', samples='by_priority')
    cfg = obj.get_config(categories=['high', 'medium', 'low'])

    yaml_str = OmegaConf.to_yaml(cfg)
    print(yaml_str)

    assert isinstance(cfg, DictConfig)
