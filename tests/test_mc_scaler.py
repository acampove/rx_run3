'''
File with functions to test MCScaler
'''
import pytest

from dmu.logging.log_store  import LogStore
from conftest               import DataCollector
from rx_misid.mc_scaler     import MCScaler

log=LogStore.add_logger('rx_misid:test_ms_scaler')
# -----------------------------------------------
class Data:
    '''
    data class
    '''
    sig_reg = '(L1_PID_E > 3) && (L2_PID_E > 3)'
# -----------------------------------------------
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC'])
def test_simple(q2bin : str, sample : str):
    '''
    Test over all samples and q2 bins
    '''
    scl = MCScaler(
            q2bin  =q2bin,
            sample =sample,
            sig_reg=Data.sig_reg)

    nsig, nctr, val = scl.get_scale()

    d_row = {
            'sample' : sample,
            'q2bin'  :  q2bin,
            'signal' :   nsig,
            'control':   nctr,
            'scale'  :    val,
             }

    DataCollector.add_entry(name='simple', data=d_row)
# -----------------------------------------------
