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
def _plot_features(df : pnd.DataFrame):
    for name in ['eng', 'row', 'col', 'are', 'npv', 'blk', 'mu']:
        if name == 'mu':
            df[name].plot.hist(bins=101, range=[0.5, 1.5])
        else:
            df[name].plot.hist(bins=100)

        plt.xlabel(name)
        plt.savefig(f'{Data.out_dir}/{name}.png')
        plt.close()
# ---------------------------------------------
def _plot_df(df : pnd.DataFrame) -> None:
    _plot_features(df=df)
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
