'''
Script containing functions meant to test MisID_PDF class
'''
import os

import matplotlib.pyplot as plt
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
    obs     = zfit.Space('B_M_brem_track_2', limits=(4500, 7000))
    out_dir = '/tmp/tests/rx_misid/misid_pdf'
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:test_misid_pdf', 10)
    LogStore.set_level('rx_misid:misid_pdf'     , 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------
def _plot_pdf(pdf : zpdf, dat : zdata, name : str) -> None:
    obj   = ZFitPlotter(data=dat, model=pdf)
    obj.plot(nbins=50)

    plt.savefig(f'{Data.out_dir}/{name}.png')
    plt.close()
# ----------------------------
def test_simple():
    '''
    Simplest test
    '''

    obj = MisIdPdf(obs=Data.obs, q2bin='central')
    pdf = obj.get_pdf()
    dat = obj.get_data()

    _plot_pdf(pdf, dat, name='simple')
# ----------------------------
