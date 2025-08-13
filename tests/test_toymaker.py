'''
This module tests the class ToyMaker
'''
from fitter.toy_maker      import ToyMaker
from dmu.stats             import utilities as sut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:test_toymaker')
# ----------------------
def test_simple() -> None:
    '''
    Simplest test of ToyMaker

    Parameters 
    -------------
    none
    '''
    nll   = sut.get_nll(kind='s+b')
    ntoys = 10

    mkr   = ToyMaker(nll=nll, ntoys=ntoys)
    df    = mkr.get_parameter_information()

    pars  = nll.get_params()
    assert len(df) == ntoys * len(pars) 
