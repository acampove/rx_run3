'''
Script used to test the DDataFrameGetter class
'''
import os
from importlib.resources   import files

import yaml
import pytest
import dask.dataframe    as DaskDataFrame
import matplotlib.pyplot as plt

from dmu.rfile.ddfgetter   import DDFGetter
from dmu.testing.utilities import build_friend_structure
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test_ddfgetter')
# ------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/rfile/ddfgetter'
    nentries= 100
# ------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# ------------------------------
# index and a,b,c,d, This is hardcoded in the utilities.py Data class
def _check_ddf(ddf : DaskDataFrame, ncol : int = 10) -> None:
    assert len(ddf)         == Data.nentries * 3 # There are 3 files in the config file
    assert len(ddf.columns) == ncol
# ------------------------------
def _plot_columns(ddf : DaskDataFrame, name : str) -> None:
    df= ddf.compute()
    df.plot.hist(range=[-3,+3], bins=20, histtype='step')
    plt.savefig(f'{Data.out_dir}/{name}.png')
# ------------------------------
def test_small():
    '''
    Tests loading trees and friends into a DaskDataFrame
    with at most 10 entries per file
    '''
    old_entries   = Data.nentries
    Data.nentries = 10

    file_name = 'friends.yaml'
    build_friend_structure(file_name=file_name, nentries=Data.nentries)
    cfg_path  = files('dmu_data').joinpath(f'rfile/{file_name}')
    with open(cfg_path, encoding='utf-8') as ofile:
        cfg = yaml.safe_load(ofile)

    ddfg = DDFGetter(cfg=cfg)
    ddf  = ddfg.get_dataframe()

    _plot_columns(ddf=ddf, name='with_conf')
    _check_ddf(ddf=ddf)

    log.info('Dataframe:')
    print(ddf.compute())

    Data.nentries = old_entries
# ------------------------------
def test_with_path():
    '''
    Tests loading trees and friends into a DaskDataFrame
    using path to config file
    '''
    file_name = 'friends.yaml'
    build_friend_structure(file_name=file_name, nentries=Data.nentries)
    cfg_path  = files('dmu_data').joinpath(f'rfile/{file_name}')

    ddfg = DDFGetter(config_path=cfg_path)
    ddf  = ddfg.get_dataframe()

    _plot_columns(ddf=ddf, name='with_path')
    _check_ddf(ddf=ddf)
# ------------------------------
def test_with_conf():
    '''
    Tests loading trees and friends into a DaskDataFrame
    using path to config file
    '''
    file_name = 'friends.yaml'
    build_friend_structure(file_name=file_name, nentries=Data.nentries)
    cfg_path  = files('dmu_data').joinpath(f'rfile/{file_name}')
    with open(cfg_path, encoding='utf-8') as ofile:
        cfg = yaml.safe_load(ofile)

    ddfg = DDFGetter(cfg=cfg)
    ddf  = ddfg.get_dataframe()

    _plot_columns(ddf=ddf, name='with_conf')
    _check_ddf(ddf=ddf)
# ------------------------------
def test_columns():
    '''
    Tests loading trees and friends into a DaskDataFrame
    with only a few columns
    '''
    file_name = 'friends.yaml'
    build_friend_structure(file_name=file_name, nentries=Data.nentries)
    cfg_path  = files('dmu_data').joinpath(f'rfile/{file_name}')

    ddfg = DDFGetter(config_path=cfg_path, columns=['a0', 'b0'])
    ddf  = ddfg.get_dataframe()

    _plot_columns(ddf=ddf, name='columns')
    _check_ddf(ddf=ddf, ncol=2)
# ------------------------------
