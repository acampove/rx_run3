'''
Script with tests for LogInfo class
'''

from ap_utilities.log_info import LogInfo

class Data:
    log_path = '/home/acampove/cernbox/dev/tests/ap_utilities/log_info/00006789.zip'

def test_simple():
    obj = LogInfo(zip_path = Data.log_path)
    nentries = obj.get_ran_entries()

    assert nentries == 13759
