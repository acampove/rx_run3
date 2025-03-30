'''
Module holding functions meant to test PDFs in zfit_models.py module
'''
import os
import zfit
import pytest
import matplotlib.pyplot as plt

from zfit.core.interfaces   import ZfitSpace as zobs
from zfit.core.basepdf      import BasePDF   as zpdf
from zfit.core.parameter    import Parameter as zpar

from dmu.stats.zfit_models  import HypExp
from dmu.stats.zfit_models  import ModExp
from dmu.stats.zfit_plotter import ZFitPlotter

# -------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/stats/zfit_models'
# -------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------
def _plot_pdf(pdf : zpdf, name : str) -> None:
    data = pdf.create_sampler(n=10000)
    obj  = ZFitPlotter(data=data, model=pdf)
    obj.plot(nbins=50)

    plt.savefig(f'{Data.out_dir}/{name}.png')
# -------------------------------
def test_hypexp():
    '''
    Test HypExp PDF
    '''
    obs= zfit.Space('x', limits=(4500, 6000))
    mu = zfit.Parameter('mu',  5000,  4000,  6000)
    ap = zfit.Parameter('ap', 0.020,     0,  0.10)
    bt = zfit.Parameter('bt', 0.002, 0.001, 0.003)

    pdf= HypExp(obs=obs, mu=mu, alpha=ap, beta=bt)

    _plot_pdf(pdf, name='hypexp')
# -------------------------------
def test_modexp():
    '''
    Test ModExp PDF
    '''
    obs= zfit.Space('x', limits=(4500, 6000))
    mu = zfit.Parameter('mu',  4500,  4000,  6000)
    ap = zfit.Parameter('ap', 0.020,     0,   0.1)
    bt = zfit.Parameter('bt', 0.002, 0.001, 0.005)

    pdf= ModExp(obs=obs, mu=mu, alpha=ap, beta=bt)

    _plot_pdf(pdf, name='modexp')
# -------------------------------
