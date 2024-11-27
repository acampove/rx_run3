'''
Script with tests for LogInfo class
'''

from ap_utilities.log_info import LogInfo

# ----------------------------
class Data:
    '''
    Class storing shared data
    '''
    log_mcdt            = '/home/acampove/cernbox/dev/tests/ap_utilities/log_info/mcdt.zip'
    log_fallback_noline = '/home/acampove/cernbox/dev/tests/ap_utilities/log_info/noline.zip'
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
def test_fallback_noline():
    '''
    Tests if the statistics used for MCDecayTree are read correctly
    '''
    obj = LogInfo(zip_path = Data.log_fallback_noline)
    nentries = obj.get_mcdt_entries('Xib_psi2SXi_ee_Lambdapi_eq_TightCut')

    assert nentries == 13603
