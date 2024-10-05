'''
Tests for RFPrinter class
'''

from dmu.rfile.rfprinter import RFPrinter

# -------------------------------------------------
class Data:
    '''
    Class used to store shared data
    '''
    file_path : str = '/publicfs/lhcb/user/campoverde/Data/RK/.run3/flt_003/dt_2024_turbo_comp_00244201_00000002_1_tuple_Hlt2RD_LbTopKMuMu_MVA.root'
# -------------------------------------------------
def test_simple():
    '''
    Test basic printing
    '''

    obj = RFPrinter(path=Data.file_path)
    obj.save(to_screen=True)
# -------------------------------------------------
