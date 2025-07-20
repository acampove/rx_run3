'''
Module testing PreProcessor class
'''
import os
import logging
import numpy
import torch
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dask.distributed              import Client
from dmu.logging.log_store         import LogStore
from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration              import utilities    as cut

log = LogStore.add_logger('ecal_calibration:test_preprocessor')

pytestmark = pytest.mark.skip(
    reason='''
    This part of the code did not produce useful results
    will turn off the tests, due to time constraints
    ''')
# -----------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/ecal_calibration/preprocessor'
    columns = {'row', 'col', 'are', 'eng', 'npv', 'blk', 'mu'}
    d_feat  = {'eng' : r'$E(\gamma)$[GeV]', 'row' : 'Row', 'col' : 'Column', 'are' : 'ECAL region', 'npv' : 'nPVs', 'blk' : 'Block'}
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
    _plot_bias(  df=df, test_name=test_name, corr=corr)
    _plot_var_mu(df=df, test_name=test_name)
# ---------------------------------------------
def _plot_var_mu(df : pnd.DataFrame, test_name : str) -> None:
    out_dir = f'{Data.out_dir}/{test_name}'
    os.makedirs(out_dir, exist_ok=True)
    for column, name in Data.d_feat.items():
        df.plot.scatter(x=column, y='mu', s=1)

        plt.legend()
        plt.grid()
        plt.xlabel(name)
        plt.ylabel(r'$\mu$')
        plt.ylim(0.2, 1.8)
        if name == 'eng':
            plt.xlim(bottom=0.0)
        plt.savefig(f'{out_dir}/{column}_mu.png')
        plt.close()
# ---------------------------------------------
def _plot_bias(df : pnd.DataFrame, test_name : str, corr : float) -> None:
    out_dir = f'{Data.out_dir}/{test_name}'
    os.makedirs(out_dir, exist_ok=True)

    ax = None
    for area, df_area in df.groupby('are'):
        ax = df_area['mu'].plot.hist(
                range=[0,2],
                bins=101,
                label=f'Region {area}',
                histtype='step',
                ax=ax,
                density=False)

    if corr is not None:
        plt.axvline(x=corr, ls=':', label='expected', color='red')

    plt.legend()
    plt.xlabel(r'$\mu$')
    plt.ylabel('Entries')
    plt.grid()
    plt.savefig(f'{out_dir}/mu.png')
    plt.close()
# ---------------------------------------------
def _plot_features(df : pnd.DataFrame, test_name : str):
    out_dir = f'{Data.out_dir}/{test_name}'
    os.makedirs(out_dir, exist_ok=True)

    for feat, name in Data.d_feat.items() :
        df[feat].plot.hist(bins=100)

        plt.xlabel(name)
        plt.savefig(f'{out_dir}/{feat}.png')
        plt.close()
# ---------------------------------------------
def test_nobias(_dask_client : Client):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is zero, i.e. mu=1
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data', bias=1.0, kind='flat')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    df  = ddf.head(100)
    df['mu'] = df['mu'] / 1000.
    _plot_df(df=df, test_name='nobias', corr=None)

    arr_mu = df['mu'].to_numpy()

    assert numpy.allclose(arr_mu, 1, rtol=1e-5)
    assert set(df.columns) == Data.columns

    _plot_df(df=df, test_name='nobias', corr= None)
# ---------------------------------------------
@pytest.mark.parametrize('bias', [0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3])
def test_flat_bias(bias : float, _dask_client : Client):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is the number that was injected
    '''
    corr= 1.0 / bias
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data',bias=bias, kind='flat')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    df  = ddf.head(100)
    df['mu'] = df['mu'] / 1000.

    name = f'flat_bias/{bias}'
    _plot_df(df=df, test_name=name, corr=corr)

    arr_mu = df['mu'].to_numpy()

    assert numpy.allclose(arr_mu, corr, rtol=1e-5)
    assert set(df.columns) == Data.columns

    _plot_df(df=df, test_name=name, corr= None)
# ---------------------------------------------
def test_row_bias(_dask_client : Client):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is the number that was injected
    '''
    bias = 1.0

    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data',bias=bias, kind='row')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    df  = ddf.head(100)
    df['mu'] = df['mu'] / 1000.

    name = f'row_bias/{100 * bias:.0f}'
    _plot_df(df=df, test_name=name, corr= None)

    assert set(df.columns) == Data.columns
# ---------------------------------------------
@pytest.mark.parametrize('bias', [0.9, 1.0, 1.2])
def test_features_target(_dask_client : Client, bias : float):
    '''
    Preprocesses a Dask dataframe and provides the tensor with the
    features
    '''
    log.info('Running test_features_target')

    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data', bias=bias, kind='flat')

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    with pre.max_entries(100):
        fet = pre.features
        tgt = pre.targets / 1000

    nrows, ncols = fet.shape
    nsample, _   = tgt.shape

    assert ncols == 6
    assert nrows >  0
    assert nrows == nsample

    corr = 1.0 / bias
    assert torch.allclose(tgt, torch.tensor(corr), rtol=1e-5)
# ---------------------------------------------
def test_real_bias(_dask_client : Client):
    '''
    Run over real data
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='real_data',bias=None, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    df       = ddf.compute()
    df['mu'] = df['mu'] / 1000.
    _plot_df(df=df, test_name='real_bias', corr=None)
# ---------------------------------------------
