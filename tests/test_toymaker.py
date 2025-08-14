'''
This module tests the class ToyMaker
'''
from fitter.toy_maker      import ToyMaker
from dmu.stats             import utilities as sut
from dmu.logging.log_store import LogStore
from dmu.stats.fitter      import Fitter

log=LogStore.add_logger('fitter:test_toymaker')
# ----------------------
def test_simple() -> None:
    '''
    Simplest test of ToyMaker

    Parameters 
    -------------
    none
    '''
    log.info('')
    nll   = sut.get_nll(kind='s+b')
    res, _= Fitter.minimize(nll=nll, cfg={})
    ntoys = 10

    mkr   = ToyMaker(nll=nll, res=res, ntoys=ntoys)
    df    = mkr.get_parameter_information()
    l_col = ['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Converged']

    assert df.columns.to_list() == l_col

    pars  = nll.get_params()
    assert len(df) == ntoys * len(pars) 
# ----------------------
