'''
Module with tests for GofCalculator class
'''
from dataclasses              import dataclass

import zfit
import numpy

from dmu.stats.gof_calculator import GofCalculator
from dmu.logging.log_store    import LogStore

log = LogStore.add_logger('dmu:stats:test_gofcalculator')
#---------------------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    obs = zfit.Space('x', limits=(-10, 10))
#---------------------------------------------
def _get_model():
    mu  = zfit.Parameter('mu', 2.4, -1, 5)
    sg  = zfit.Parameter('sg', 1.3,  0, 5)
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
    Simplest test of GofCalculator
    '''
    nll = _get_nll()

    gcl = GofCalculator(nll)
    gof = gcl.get_gof(kind='pvalue')
# -------------------------------------------
