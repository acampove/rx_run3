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
    file_name = '/tmp/tuple_reader_test.root'
    tree_name = 'DecayTree'
# -------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    if os.path.isfile(Data.file_name):
        return

    rdf = RDataFrame(100)
    rdf = rdf.Define('a', '1')
    rdf = rdf.Define('b', '2')

    rdf.Snapshot(Data.tree_name, Data.file_name)
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
    trd = TupleReader(Data.tree_name, Data.file_name)
    trd.Init()
    tup = trd.Tuple()
    tup.Print()
# -------------------------------------
