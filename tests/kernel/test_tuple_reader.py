'''
Module containing tests for TupleReader
'''
from dataclasses import dataclass
import pytest

from rx_kernel.tuple_reader import TupleReader
from rx_common              import utilities   as ut

from ROOT import MessageSvc


MessageSvc.Initialize(-1)

# -------------------------------------
@dataclass
class Data:
    '''
    Class used to share variables
    '''
    cfg_inp  = {
            'nfiles'  : 10,
            'nentries': 100,
            'data_dir': '/tmp/test_tuple_reader',
            'sample'  : 'data_24_magdown_24c4',
            'hlt2'    : 'Hlt2RD_BuToKpEE_MVA'}
# -------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    ut.make_inputs(Data.cfg_inp)
# -------------------------------------
def test_default():
    '''
    Tests default constructor
    '''
    trd = TupleReader()
# -------------------------------------
def test_file_tuple():
    '''
    This will test the constructor taking file path and tree name
    '''
    file_path = f'{Data.cfg_inp["data_dir"]}/{Data.cfg_inp["sample"]}/{Data.cfg_inp["hlt2"]}/file_000.root'

    trd = TupleReader('DecayTree', file_path)
    trd.Init()
    tup = trd.Tuple()
    tup.Print()
# -------------------------------------
