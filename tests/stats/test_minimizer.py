'''
Script used to test custom zfit minimizer
'''
from dataclasses import dataclass

import zfit
import numpy
import pytest

from dmu.stats.minimizers  import AnealingMinimizer
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:test_minimizer')
#---------------------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    obs = zfit.Space('x', limits=(-10, 10))
#---------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:ml:minimizers', 10)
#---------------------------------------------
def _get_model():
    mu  = zfit.Parameter('mu', 5.0, -1, 5)
    sg  = zfit.Parameter('sg', 5.0,  0, 5)
    pdf = zfit.pdf.Gauss(obs=Data.obs, mu=mu, sigma=sg)

    return pdf
# -------------------------------------------
def _get_data():
    data_np = numpy.random.normal(0, 1, size=10000)
    data_zf = zfit.Data.from_numpy(obs=Data.obs, array=data_np)

    return data_zf
# -------------------------------------------
def _get_nll():
    pdf = _get_model()
    dat = _get_data()
    nll = zfit.loss.UnbinnedNLL(model=pdf, data=dat)

    return nll
# -------------------------------------------
def test_simple():
    '''
    SImplest test of minimizer
    '''
    nll       = _get_nll()
    minimizer = AnealingMinimizer(ntries=10, pvalue=0.15)
    res       = minimizer.minimize(nll)

    print(res)
# -------------------------------------------
