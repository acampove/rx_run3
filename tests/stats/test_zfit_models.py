'''
Module holding functions meant to test PDFs in zfit_models.py module
'''
import os
import pytest
import zfit
from zfit.core.interfaces   import ZfitSpace as zobs
from zfit.core.basepdf      import BasePDF   as zpdf
from zfit.core.parameter    import Parameter as zpar

from stats.zfit_models      import HypExp
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
    os.mkdir(Data.out_dir, exist_ok=True)
# -------------------------------
def _plot_pdf(pdf : zpdf, name : str) -> None:
    data = pdf.create_sampler(n=10000)
    obj  = ZFitPlotter(data=data, model=pdf)
    obj.plot(nbins=50)

    plt.savefig(f'{Data.out_dir}/{name}.pdf')
# -------------------------------
def test_hypexp():
    '''
    Test HypExp PDF
    '''
    obs= zfit.Space('x', limits=(4500, 6000))
    mu = zfit.Parameter('mu',   5000, 4000, 6000)
    ap = zfit.Parameter('ap',    200,    0, 1000)
    bt = zfit.Parameter('bt', 0.0001, 0.00, 0.01)

    pdf= HypExp(obs=obs, mu=mu, ap=ap, bt=bt)

    _plot_pdf(pdf, name='hypexp')
# -------------------------------
