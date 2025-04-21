'''
Script meant to test MisIDCalculator class
'''
from importlib.resources import files

import yaml
import pytest
from dmu.logging.log_store     import LogStore
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:test_misid_calculator')
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:misid_calculator', 10)
    LogStore.set_level('rx_misid:splitter'        , 10)
    LogStore.set_level('rx_misid:weighter'        , 10)
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

    # data  : DATA_*
    # signal: Bu_Kee_eq_btosllball05_DPC
    # leak  : Bu_JpsiK_ee_eq_DPC
    cfg = _get_config()
    cfg['input']['max_entries'] = 100_000
    cfg['input']['sample'     ] = 'DATA_*'
    cfg['input']['q2bin'      ] = 'jpsi'

    obj =MisIDCalculator(cfg=cfg)
    data=obj.get_misid()

    print(data)
# ---------------------------------
