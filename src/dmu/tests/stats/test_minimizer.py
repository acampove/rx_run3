'''
Script used to test custom zfit minimizer
'''
import numpy
import pytest

from dmu        import LogStore
from dmu.stats  import zfit
from dmu.stats  import AnealingMinimizer
from zfit.data  import Data as zdat

log = LogStore.add_logger('dmu:ml:test_minimizer')
#---------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    obs = zfit.Space('x', limits=(-10, 10))
#---------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    LogStore.set_level('dmu:ml:minimizers', 10)
#---------------------------------------------
def _get_model():
    mu  = zfit.Parameter('mu', 5.0, -1, 5)
    sg  = zfit.Parameter('sg', 5.0,  0, 5)
    ar  = zfit.Parameter('ar_dscb', 1,    0,    5)
    al  = zfit.Parameter('al_dscb', 1,    0,    5)
    nr  = zfit.Parameter('nr_dscb', 2,    1,    5)
    nl  = zfit.Parameter('nl_dscb', 2,    0,    5)

    pdf = zfit.pdf.DoubleCB(mu, sg, al, nl, ar, nr, Data.obs)

    return pdf
# -------------------------------------------
def _get_data() -> zdat:
    numpy.random.seed(42)
    data_1  = numpy.random.normal(0, 1.0, size=15_000)
    data_2  = numpy.random.normal(0, 1.2, size= 1_500)
    data_np = numpy.concatenate([data_1, data_2])

    data    = zfit.Data.from_numpy(obs=Data.obs, array=data_np)

    if not isinstance(data, zdat):
        raise ValueError('Data not a zfit data instance')

    return data
# -------------------------------------------
def _get_nll():
    pdf = _get_model()
    dat = _get_data()
    nll = zfit.loss.UnbinnedNLL(model=pdf, data=dat)

    return nll
# -------------------------------------------
def test_pvalue():
    '''
    SImplest test of minimizer with pvalue threshold
    '''
    nll       = _get_nll()
    minimizer = AnealingMinimizer(ntries=10, pvalue=0.05)
    res       = minimizer.minimize(nll)

    print(res)
# -------------------------------------------
def test_chi2ndof():
    '''
    SImplest test of minimizer with pvalue threshold
    '''
    nll       = _get_nll()
    minimizer = AnealingMinimizer(ntries=10, chi2ndof=1.00)
    res       = minimizer.minimize(nll)

    print(res)
# -------------------------------------------
