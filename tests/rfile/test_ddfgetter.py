'''
Script used to test the DDataFrameGetter class
'''
import os
from importlib.resources   import files

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
# ------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# ------------------------------
def _plot_columns(ddf : DaskDataFrame, name : str) -> None:
    df= ddf.compute()
    df= df.drop(columns=['index'])
    df.plot.hist(range=[-3,+3], bins=20, histtype='step')
    plt.savefig(f'{Data.out_dir}/{name}.png')
# ------------------------------
def test_simple():
    '''
    Simplest test for loading trees and friends into a DaskDataFrame
    '''
    file_name = 'friends.yaml'
    build_friend_structure(file_name=file_name)
    cfg_path  = files('dmu_data').joinpath(f'rfile/{file_name}')

    ddfg = DDFGetter(config_path=cfg_path)
    ddf  = ddfg.get_dataframe()

    _plot_columns(ddf=ddf, name='simple')
# ------------------------------
