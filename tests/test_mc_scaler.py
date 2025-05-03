'''
File with functions to test MCScaler
'''
import pytest

from dmu.logging.log_store  import LogStore
from conftest               import DataCollector
from rx_misid.mc_scaler     import MCScaler
from rx_misid.misid_pdf     import MisIdPdf

log=LogStore.add_logger('rx_misid:test_ms_scaler')
# -----------------------------------------------
class Data:
    '''
    data class
    '''
    version = 'v1' # This should be the version of the config actually used in the analysis
# -----------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:test_ms_scaler', 10)
    LogStore.set_level('rx_misid:ms_scaler'     , 10)
# -----------------------------------------------
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC'])
def test_simple(q2bin : str, sample : str):
    '''
    Test over all samples and q2 bins
    '''
    sig_reg = MisIdPdf.get_signal_cut(Data.version)

    scl = MCScaler(
            q2bin  =q2bin,
            sample =sample,
            sig_reg=sig_reg)

    nsig_mc, nctr_mc, scl = scl.get_scale()

    d_row = {
            'sample' :  sample,
            'q2bin'  :   q2bin,
            'sig_mc' : nsig_mc,
            'ctr_mc' : nctr_mc,
            'scale'  :     scl,
             }

    DataCollector.add_entry(name='simple', data=d_row)
# -----------------------------------------------
