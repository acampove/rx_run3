'''
File with tests for PIDWeigher class
'''
import os
import pytest
import matplotlib.pyplot as plt

from dmu.stats.zfit         import zfit

from dmu.logging.log_store  import LogStore
from dmu.stats.zfit_plotter import ZFitPlotter
from zfit.core.interfaces   import ZfitPDF    as zpdf
from zfit.core.interfaces   import ZfitData   as zdata
from rx_misid.pdf_maker     import PDFMaker

log=LogStore.add_logger('rx_misid:test_pdf_maker')
# ------------------------------------
class Data:
    '''
    Stores shared attributes
    '''
    obs     = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
    out_dir = '/tmp/tests/rx_misid/pdf_maker'
# ------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter'       , 10)
    LogStore.set_level('rx_misid:pdf_maker'       , 10)
    LogStore.set_level('rx_misid:misid_calculator', 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ------------------------------------
def _check_pdf(
        pdf    : zpdf,
        q2bin  : str,
        data   : zdata,
        sample : str,
        region : str) -> None:
    '''
    Take zfit pdf and validate it
    '''
    ext_text = f'{sample}\n{region}\n{q2bin}'

    obj= ZFitPlotter(data=data, model=pdf)
    obj.plot(ext_text=ext_text)

    plot_path = f'{Data.out_dir}/{sample}_{region}_{q2bin}.png'
    log.info(f'Saving to: {plot_path}')

    plt.savefig(plot_path)
    plt.close()
# ------------------------------------
#'Bu_KplKplKmn_eq_sqDalitz_DPC',   # To be uncommented once MC be reprocessed
@pytest.mark.parametrize('sample', [
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_JpsiK_ee_eq_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC'])
@pytest.mark.parametrize('is_sig', [True, False])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
def test_simple(sample : str, q2bin : str, is_sig : bool):
    '''
    Simplest test
    '''
    mkr = PDFMaker(sample=sample, q2bin=q2bin, trigger=Data.trigger)
    pdf = mkr.get_pdf(obs=Data.obs, is_sig=is_sig)

    region = {True : 'signal', False : 'control'}[is_sig]

    _check_pdf(
        pdf    = pdf,
        q2bin  = q2bin,
        data   = pdf.dat,
        sample = sample,
        region = region)
# ------------------------------------
