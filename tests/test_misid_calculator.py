'''
Script meant to test MisIDCalculator class
'''
import os
from importlib.resources import files

import yaml
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from dmu.logging.log_store     import LogStore
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:test_misid_calculator')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir     = '/tmp/tests/rx_misid/misid_calculator'
    config_name = 'misid.yaml'
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:misid_calculator', 10)
    LogStore.set_level('rx_misid:splitter'        , 10)
    LogStore.set_level('rx_misid:weighter'        , 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ---------------------------------
def _get_config() -> dict:
    config_path = files('rx_misid_data').joinpath(Data.config_name)
    config_path = str(config_path)
    with open(config_path, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    return data
# ---------------------------------
def _get_sample(name : str) -> str:
    if name == 'data':
        return 'DATA_24_MagUp_24c3'

    if name == 'signal':
        return 'Bu_Kee_eq_btosllball05_DPC'

    if name == 'leakage':
        return 'Bu_JpsiK_ee_eq_DPC'

    return name
# ---------------------------------
def _validate_df(df : pnd.DataFrame, sample : str) -> None:
    ax = None
    ax = df.plot.hist('B_Mass_smr'      , bins=50, histtype='step', range=(4500, 6000), ax=ax)
    ax = df.plot.hist('B_M_brem_track_2', bins=50, histtype='step', range=(4500, 6000), ax=ax)

    plt.savefig(f'{Data.out_dir}/{sample}.png')
    plt.close()
# ---------------------------------
@pytest.mark.parametrize('name', ['data', 'signal', 'leakage'])
def test_sample(name : str):
    '''
    Simplest example of misid calculator with different samples
    '''
    cfg                    = _get_config()
    cfg['input']['sample'] = _get_sample(name=name)
    cfg['input']['q2bin' ] = 'central'

    obj = MisIDCalculator(cfg=cfg)
    df  = obj.get_misid()

    _validate_df(df=df, sample=name)
# ---------------------------------
@pytest.mark.parametrize('sample', [
    #'Bu_KplKplKmn_eq_sqDalitz_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC'])
def test_misid(sample : str):
    '''
    Test calculator with misID samples, for noPID trigger
    '''

    cfg                     = _get_config()
    cfg['input']['sample' ] = sample
    cfg['input']['q2bin'  ] = 'central'
    cfg['input']['project'] = 'nopid'
    cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_noPID'

    obj = MisIDCalculator(cfg=cfg)
    df  = obj.get_misid()

    _validate_df(df=df, sample=sample)
# ---------------------------------
