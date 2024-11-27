'''
Script with tests for LogInfo class
'''

from ap_utilities.log_info import LogInfo

# ----------------------------
class Data:
    '''
    Class storing shared data
    '''
    log_path = '/home/acampove/cernbox/dev/tests/ap_utilities/log_info/00006789.zip'
# ----------------------------
def test_mcdt():
    '''
    Tests if the statistics used for MCDecayTree are read correctly
    '''
    obj = LogInfo(zip_path = Data.log_path)
    nentries = obj.get_mcdt_entries('Bu_Kee_eq_btosllball05_DPC')

    assert nentries == 13584
