'''
Module holding functions meant to test PDFs in zfit_models.py module
'''
import os
import zfit
import pytest
import matplotlib.pyplot as plt

from zfit.core.basepdf      import BasePDF   as zpdf
from dmu.stats.zfit_models  import HypExp
from dmu.stats.zfit_models  import ModExp
from dmu.stats.zfit_plotter import ZFitPlotter

# -------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/stats/zfit_models'

    l_mu_mod_exp = [
            4000,
            4100,
            4200,
            4300,
            4500,
            4600,
            ]
# -------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------
def _plot_pdf(pdf : zpdf, name : str, mu_val : float) -> None:
    data = pdf.create_sampler(n=1_000)
    obj  = ZFitPlotter(data=data, model=pdf)
    obj.plot(nbins=50, title=f'$\\mu={{{mu_val}}}$')

    out_dir = f'{Data.out_dir}/{name}'
    os.makedirs(out_dir, exist_ok=True)

    plt.savefig(f'{out_dir}/{mu_val}.png')
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
@pytest.mark.parametrize('mu_val', Data.l_mu_mod_exp)
def test_modexp(mu_val : int):
    '''
    Test ModExp PDF
    '''
    obs= zfit.Space('x', limits=(4500, 6000))
    mu = zfit.Parameter('mu', mu_val,  4000,  5000)
    ap = zfit.Parameter('ap',  0.020,     0,   0.1)
    bt = zfit.Parameter('bt',  0.002, 0.001, 0.005)

    pdf= ModExp(obs=obs, mu=mu, alpha=ap, beta=bt)

    _plot_pdf(pdf, name='modexp', mu_val=mu_val)
# -------------------------------
