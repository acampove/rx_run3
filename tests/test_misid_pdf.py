'''
Script containing functions meant to test MisID_PDF class
'''
import os

import matplotlib.pyplot as plt
import pandas            as pnd
import pytest
import zfit
from zfit.core.data         import Data       as zdata
from zfit.core.basepdf      import BasePDF    as zpdf

from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.logging.log_store  import LogStore
from rx_misid.misid_pdf     import MisIdPdf

log=LogStore.add_logger('rx_misid:test_misid_pdf')
# ----------------------------
class Data:
    '''
    Data class
    '''
    obs_name= 'B_M_brem_track_2'
    obs     = zfit.Space(obs_name, limits=(4500, 7000))
    out_dir = '/tmp/tests/rx_misid/misid_pdf'
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:test_misid_pdf', 10)
    LogStore.set_level('rx_misid:misid_pdf'     , 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------
def _plot_data(df : pnd.DataFrame, q2bin : str) -> None:
    ax = None
    for sample, df_sample in df.groupby('sample'):
        ax = df_sample[Data.obs_name].plot.hist(column=Data.obs_name, range=[4500, 7000], bins=60, histtype='step', weights=df_sample['weight'], label=sample, ax=ax)

    plt.legend()
    plt.title(q2bin)
    plt.savefig(f'{Data.out_dir}/{q2bin}.png')
    plt.close()
# ----------------------------
def _plot_pdf(pdf  : zpdf,
              dat  : zdata,
              name : str,
              q2bin: str) -> None:
    obj   = ZFitPlotter(data=dat, model=pdf)
    obj.plot(nbins=50)
    obj.axs[0].axhline(y=+0, color='gray', linestyle=':')
    obj.axs[1].axhline(y=-3, color='red' , linestyle=':')
    obj.axs[1].axhline(y=+3, color='red' , linestyle=':')
    obj.axs[1].set_ylim(-5, 5)

    out_dir = f'{Data.out_dir}/{name}'
    os.makedirs(out_dir, exist_ok=True)

    plt.savefig(f'{out_dir}/{q2bin}.png')
    plt.close()
# ----------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_pdf(q2bin : str):
    '''
    Tests PDF provided by tool
    '''

    obj = MisIdPdf(obs=Data.obs, q2bin=q2bin)
    pdf = obj.get_pdf()
    dat = obj.get_data(kind='zfit')

    _plot_pdf(pdf, dat, name='simple', q2bin=q2bin)
# ----------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_data(q2bin : str):
    '''
    Tests that the tool can provide data
    '''

    obj = MisIdPdf(obs=Data.obs, q2bin=q2bin)
    df  = obj.get_data()

    _plot_data(df, q2bin)
# ----------------------------
