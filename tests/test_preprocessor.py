'''
Module testing PreProcessor class
'''
import os

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dask.dataframe                import DataFrame    as DDF
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
# ---------------------------------------------
def _inject_bias(ddf : DDF, bias : float) -> DDF:
    '''
    This function scales the momentum components of the lepton by the `bias` factor
    This is done only when the electrons have brem associated, i.e. L*_brem == 1
    '''
    for lep in ['L1', 'L2']:
        ddf[f'{lep}_PT'] = ddf[f'{lep}_PT'] + ddf[f'{lep}_PT'] * ddf[f'{lep}_brem'] * (bias - 1)

    return ddf
# ---------------------------------------------
def _plot_features(df : pnd.DataFrame, plot_name : str):
    for feature in ['eng', 'row', 'col', 'are', 'npv', 'blk']:
        df[feature].plot.hist(bins=100)

        plt.xlabel(feature)
        plt.savefig(f'{Data.out_dir}/{plot_name}_{feature}.png')
        plt.close()
# ---------------------------------------------
def _plot_df(
        df   : pnd.DataFrame,
        name : str,
        corr : float) -> None:
    _plot_features(df=df, plot_name=name)
    _plot_bias(df=df, plot_name=name, corr=corr)
# ---------------------------------------------
def _plot_bias(df : pnd.DataFrame, plot_name : str, corr : float) -> None:
    df['mu'].plot.hist(bins=101, range=[0.5, 1.5], label='measured')
    plt.axvline(x=corr, ls=':', label='expected', color='red')

    plt.legend()
    plt.savefig(f'{Data.out_dir}/mu_{plot_name}.png')
    plt.close()
# ---------------------------------------------
def test_nobias():
    '''
    Tests that:

    - The features can be retrieved
    - The bias is zero, i.e. mu=1
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf()

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    df  = ddf.compute()
    _plot_df(df=df)

    arr_mu = df['mu'].to_numpy()

    assert numpy.allclose(arr_mu, 1, rtol=1e-5)
    assert set(df.columns) == Data.columns
# ---------------------------------------------
@pytest.mark.parametrize('bias', [0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3])
def test_flat_bias(bias : float):
    '''
    Tests that:

    - The features can be retrieved
    - The bias is the number that was injected
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf()
    ddf = _inject_bias(ddf, bias)

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    df  = ddf.compute()

    name = f'flat_bias_{100 * bias:.0f}'
    _plot_df(df=df, name=name, corr= 1./bias)

    arr_mu = df['mu'].to_numpy()

    assert numpy.allclose(arr_mu, 1 / bias, rtol=1e-5)
    assert set(df.columns) == Data.columns
# ---------------------------------------------
