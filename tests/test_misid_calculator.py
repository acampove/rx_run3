'''
Script meant to test MisIDCalculator class
'''
import os
from importlib.resources import files

import yaml
import pytest
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
@pytest.mark.parametrize('sample', ['DATA_*', 'Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC'])
def test_sample(sample : str):
    '''
    Simplest example of misid calculator with different samples
    '''
    cfg                    = _get_config()
    if 'DATA' in sample:
        cfg['input']['range' ] = 600_000, 610_000
    cfg['input']['sample'] = sample
    cfg['input']['q2bin' ] = 'jpsi'

    obj = MisIDCalculator(cfg=cfg)
    df  = obj.get_misid()

    sample = sample.replace('_*', '')

    df.to_parquet(f'{Data.out_dir}/misid_{sample}.parquet')
# ---------------------------------
