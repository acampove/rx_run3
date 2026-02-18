'''
Script used to test custom zfit minimizers
'''
import pytest

from dmu         import LogStore
from dmu.stats   import AnealingMinimizer
from dmu.stats   import ContextMinimizer
from dmu.stats   import zfit
from dmu.testing import get_nll
from dmu.testing import SumLiteral

log = LogStore.add_logger('dmu:ml:test_minimizer')
#---------------------------------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    LogStore.set_level('dmu:ml:minimizers', 10)
#---------------------------------------------
def test_anealing_pval():
    '''
    Test AnealingMinimizer with pvalue
    '''
    nll       = get_nll(kind = 's+b')
    minimizer = AnealingMinimizer()
    res       = minimizer.get_result(nll)

    print(res)
# -------------------------------------------
def test_anealing_chi2():
    '''
    Test AnealingMinimizer with chi2
    '''
    nll       = get_nll(kind = 's+b')
    minimizer = AnealingMinimizer()
    res       = minimizer.get_result(nll)

    print(res)
# -------------------------------------------
@pytest.mark.parametrize('kind', ['s+b'])
def test_context_minimizer(kind : SumLiteral):
    '''
    Test context minimizer 
    '''
    nll       = get_nll(kind = kind)
    min       = zfit.minimize.Minuit()

    minimizer = ContextMinimizer(min = min)
    res       = minimizer.minimize(loss = nll)

    print(res)
# -------------------------------------------
def test_context_minimizer_sim():
    '''
    Test context minimizer with simultaneous fit
    '''
    nll_ee    = get_nll(kind = 's+b', suffix = 'ee')
    nll_mm    = get_nll(kind = 's+b', suffix = 'mm')
    nll       = nll_ee + nll_mm

    min       = zfit.minimize.Minuit()
    minimizer = ContextMinimizer(min = min)
    res       = minimizer.minimize(loss = nll)

    print(res)
# -------------------------------------------
