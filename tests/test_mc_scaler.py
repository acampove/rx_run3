'''
File with functions to test MCScaler
'''
from importlib.resources import files
import pytest
import yaml

from dmu.logging.log_store  import LogStore
from conftest               import DataCollector
from rx_misid.mc_scaler     import MCScaler

log=LogStore.add_logger('rx_misid:test_ms_scaler')
# -----------------------------------------------
class Data:
    '''
    data class
    '''
    version = 'v1' # This should be the version of the config actually used in the analysis
# -----------------------------------------------
def _get_signal_cut() -> str:
    config_path = files('rx_misid_data').joinpath(f'misid_{Data.version}.yaml')
    with open(config_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    cut = cfg['splitting']['lepton_tagging']['pass']

    log.info(f'Using signal cut: {cut}')

    cut_l1 = cut.replace('LEP', 'L1')
    cut_l2 = cut.replace('LEP', 'L2')

    return f'({cut_l1}) && ({cut_l2})'
# -----------------------------------------------
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC'])
def test_simple(q2bin : str, sample : str):
    '''
    Test over all samples and q2 bins
    '''
    sig_reg = _get_signal_cut()

    scl = MCScaler(
            q2bin  =q2bin,
            sample =sample,
            sig_reg=sig_reg)

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
