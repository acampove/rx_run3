'''
Script used to test custom zfit minimizer
'''
from dataclasses import dataclass

import zfit
import numpy

from dmu.stats.minimizers import AnealingMinimizer

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
    SImplest test of minimizer
    '''
    nll       = _get_nll()
    minimizer = AnealingMinimizer(ntries=10, pvalue=0.05)
    #minimizer = zfit.minimize.Minuit()
    res       = minimizer.minimize(nll)

    print(res)
# -------------------------------------------
