'''
Script holding functions needed to test SampleWeighter class
'''
from importlib.resources import files

import yaml
import numpy
import pytest
import pandas as pnd
from dmu.logging.log_store   import LogStore
from rx_misid.weighter       import SampleWeighter

log=LogStore.add_logger('rx_misid:test_weighter')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    nentries = 10_000

    l_block = [
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
            ]
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:weighter', 10)
# -------------------------------------------------------
def _get_config() -> dict:
    cfg_path = files('rx_misid_data').joinpath('config.yaml')
    log.info(f'Picking up config from: {cfg_path}')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg['weights']
# ----------------------------
def _get_dataframe() -> pnd.DataFrame:
    df           = pnd.DataFrame(index=range(Data.nentries))
    df['hadron'] = numpy.random.choice(['kaon' ,   'pion'], size=Data.nentries)
    df['bmeson'] = numpy.random.choice(['bplus', 'bminus'], size=Data.nentries)
    df['kind'  ] = numpy.random.choice(['PassFail', 'FailPass', 'FailFail'], size=Data.nentries)
    df['block' ] = numpy.random.choice(Data.l_block, size=Data.nentries)

    for lep in ['L1', 'L2']:
        df[f'{lep}_TRACK_P'  ] = numpy.random.uniform(0, 20_000, Data.nentries)
        df[f'{lep}_TRACK_ETA'] = numpy.random.uniform(1.5,    5, Data.nentries)

    return df
# ----------------------------
def test_simple():
    '''
    Simplest test
    '''
    cfg = _get_config()
    df  = _get_dataframe()

    wgt = SampleWeighter(df=df, cfg=cfg)
    df  = wgt.get_weighted_data()

    print(df.weights)
# ----------------------------
