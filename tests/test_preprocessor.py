'''
Module testing PreProcessor class
'''
import os
import logging
import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dask.distributed              import Client
from dmu.logging.log_store         import LogStore
from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration              import utilities    as cut

log = LogStore.add_logger('ecal_calibration:test_preprocessor')
# -----------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/ecal_calibration/preprocessor'
    columns = {'row', 'col', 'are', 'eng', 'npv', 'blk', 'mu'}
# -----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
    logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)
    logging.getLogger("matplotlib.font_manager").setLevel(logging.WARNING)
# ---------------------------------------------
def _plot_df(
        df        : pnd.DataFrame,
        test_name : str,
        corr      : float) -> None:

    _plot_features(df=df, test_name=test_name)
    _plot_bias(    df=df, test_name=test_name, corr=corr)
# ---------------------------------------------
def _plot_bias(df : pnd.DataFrame, test_name : str, corr : float) -> None:
    df['mu'].plot.hist(bins=101, range=[0.5, 1.5], label='measured')
    if corr is not None:
        plt.axvline(x=corr, ls=':', label='expected', color='red')

    out_dir = f'{Data.out_dir}/{test_name}'
    os.makedirs(out_dir, exist_ok=True)

    plt.legend()
    plt.savefig(f'{out_dir}/mu.png')
    plt.close()
# ---------------------------------------------
def _plot_features(df : pnd.DataFrame, test_name : str):
    out_dir = f'{Data.out_dir}/{test_name}'
    os.makedirs(out_dir, exist_ok=True)

    for feature in ['eng', 'row', 'col', 'are', 'npv', 'blk']:
        df[feature].plot.hist(bins=100)

        plt.xlabel(feature)
        plt.savefig(f'{out_dir}/{feature}.png')
        plt.close()
# ---------------------------------------------
def test_nobias(_dask_client : Client):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is zero, i.e. mu=1
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(bias=1.0, kind='flat')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    df  = ddf.compute()
    _plot_df(df=df, test_name='nobias', corr=None)

    arr_mu = df['mu'].to_numpy()

    assert numpy.allclose(arr_mu, 1, rtol=1e-5)
    assert set(df.columns) == Data.columns
# ---------------------------------------------
@pytest.mark.parametrize('bias', [0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3])
def test_flat_bias(bias : float, _dask_client : Client):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is the number that was injected
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(bias=bias, kind='flat')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    df  = ddf.head(100)

    name = f'flat_bias/{100 * bias:.0f}'
    _plot_df(df=df, test_name=name, corr= 1./bias)

    arr_mu = df['mu'].to_numpy()

    assert numpy.allclose(arr_mu, 1 / bias, rtol=1e-5)
    assert set(df.columns) == Data.columns
# ---------------------------------------------
def test_row_bias(_dask_client : Client):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is the number that was injected
    '''
    bias = 1.0

    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(bias=bias, kind='row')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    df  = ddf.head(100)

    name = f'row_bias/{100 * bias:.0f}'
    _plot_df(df=df, test_name=name, corr= None)

    assert set(df.columns) == Data.columns
# ---------------------------------------------
def test_features(_dask_client : Client):
    '''
    Preprocesses a Dask dataframe and provides the tensor with the
    features
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(bias=1.0, kind='flat')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    fet = pre.features

# ---------------------------------------------
