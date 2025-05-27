'''
Module testing PreProcessor class
'''
import os
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

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
def test_simple():
    '''
    Simplest test for preprocessor
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf()

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    df  = ddf.compute()
    _plot_df(df=df)

    for column in df.columns:
        log.info(column)
# ---------------------------------------------
