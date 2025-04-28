'''
Script meant to test MisIDCalculator class
'''
import os
from importlib.resources import files

import yaml
import pytest
import pandas as pnd
from dmu.logging.log_store     import LogStore
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:test_misid_calculator')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/rx_misid/misid_calculator'
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:misid_calculator', 10)
    LogStore.set_level('rx_misid:splitter'        , 10)
    LogStore.set_level('rx_misid:weighter'        , 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ---------------------------------
def _get_config() -> dict:
    config_path = files('rx_misid_data').joinpath('config.yaml')
    with open(config_path, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    return data
# ---------------------------------
def _get_sample(name : str) -> str:
    if name == 'data':
        return 'DATA_24_MagUp_24c1'

    return name
# ---------------------------------
@pytest.mark.parametrize('name', ['data', 'Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC'])
def test_sample(name : str):
    '''
    Simplest example of misid calculator with different samples
    '''
    cfg                    = _get_config()
    cfg['input']['sample'] = _get_sample(name=name)
    cfg['input']['q2bin' ] = 'central'

    obj = MisIDCalculator(cfg=cfg)
    df  = obj.get_misid()

    df.to_parquet(f'{Data.out_dir}/misid_{name}.parquet')
# ---------------------------------
