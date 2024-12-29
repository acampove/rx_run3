'''
Module containing tests for TupleReader
'''
# pylint: disable=import-error, wrong-import-order

from dataclasses import dataclass
import pytest

from rx_kernel                import MessageSvc
from rx_kernel.tuple_reader   import TupleReader
from rx_kernel.test_utilities import make_inputs


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
    make_inputs(is_run3=True)
# -------------------------------------
def test_default():
    '''
    Tests default constructor
    '''
    trd = TupleReader()
    trd.PrintInline()
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
def test_add_files():
    '''
    Will test TupleReader with wildcard defining paths to files
    '''
    file_path = f'{Data.cfg_inp["data_dir"]}/{Data.cfg_inp["sample"]}/{Data.cfg_inp["hlt2"]}/file_*.root'

    trd=TupleReader('DecayTree')
    trd.AddFiles(file_path)
    trd.Init()
# -------------------------------------
