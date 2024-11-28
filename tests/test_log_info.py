'''
Script with tests for LogInfo class
'''
import pytest

from ap_utilities.logfiles.log_info import LogInfo

# ----------------------------
class Data:
    '''
    Class storing shared data
    '''
    log_mcdt            = '/home/acampove/cernbox/dev/tests/ap_utilities/log_info/mcdt.zip'

    l_log_fallback_noline = [
    ('/home/acampove/cernbox/dev/tests/ap_utilities/log_info/noline.zip'         , 'Xib_psi2SXi_ee_Lambdapi_eq_TightCut'         , 13603),
    ('/home/acampove/cernbox/dev/tests/ap_utilities/log_info/fall_back_omega.zip', 'Omegab_JpsiOmega_mm_LambdaK_eq_phsp_TightCut', 13998),
    ]
# ----------------------------
def test_mcdt():
    '''
    Tests if the statistics used for MCDecayTree are read correctly
    '''
    obj = LogInfo(zip_path = Data.log_mcdt)
    nentries = obj.get_mcdt_entries('Bu_Kee_eq_btosllball05_DPC')

    assert nentries == 13584
# ----------------------------
def test_fallback_nofile():
    '''
    Tests if the statistics used for MCDecayTree are read correctly
    '''
    obj = LogInfo(zip_path = '/path/that/does/not/exist.zip')
    nentries = obj.get_mcdt_entries('Bu_Kee_eq_btosllball05_DPC')

    assert nentries == -1
# ----------------------------
@pytest.mark.parametrize('zip_path, sample, entries', Data.l_log_fallback_noline)
def test_fallback_noline(zip_path : str, sample : str, entries : int):
    '''
    Tests if the statistics used for MCDecayTree are read correctly
    '''
    obj = LogInfo(zip_path = zip_path)
    nentries = obj.get_mcdt_entries(sample)

    assert nentries == entries
