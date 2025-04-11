'''
Module holding functions meant to test PDFs in zfit_models.py module
'''
import os
import zfit
import numpy
import mplhep
import pytest
import matplotlib.pyplot as plt

from zfit.core.basepdf      import BasePDF   as zpdf
from dmu.stats.zfit_models  import HypExp
from dmu.stats.zfit_models  import ModExp

# -------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/stats/zfit_models'

    l_ap_mod_exp = numpy.arange(0.002, 0.028, 0.002)
    l_bt_mod_exp = numpy.arange(0.002, 0.022, 0.002)
    l_mu_mod_exp = numpy.arange( 4250,  4500,    20)

    npoint =  300
    minx   = 4500
    maxx   = 6000
    obs    = zfit.Space('x', limits=(minx, maxx))
# -------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
    mplhep.style.use('LHCb2')
# -------------------------------
def _plot_pdf(pdf : zpdf, par_val : float) -> None:
    arr_x   = numpy.linspace(Data.minx, Data.maxx, Data.npoint)
    arr_y   = pdf.pdf(arr_x)
    par_str = f'{par_val:.3f}'

    if not plt.get_fignums():
        plt.figure(figsize=(15,10))

    plt.plot(arr_x, arr_y, label=par_str)
# -------------------------------
def test_modexp_mu_scan():
    '''
    Test ModExp PDF scan over mu parameter
    '''
    min_mu = Data.l_mu_mod_exp[ 0]
    max_mu = Data.l_mu_mod_exp[-1]

    mu = zfit.Parameter('mu', min_mu, min_mu, max_mu)
    ap = zfit.Parameter('ap',   0.01,   0.01,   0.02)
    bt = zfit.Parameter('bt',  0.002,  0.001,  0.005)
    pdf= ModExp(obs=Data.obs, mu=mu, alpha=ap, beta=bt)

    for mu_val in Data.l_mu_mod_exp:
        mu.set_value(mu_val)
        _plot_pdf(pdf, par_val=mu_val)

    plt.title(r'$\mu$')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/mu_scan.png')
    plt.close()
# -------------------------------
def test_modexp_ap_scan():
    '''
    Test ModExp PDF scan over alpha parameter
    '''
    min_ap = Data.l_ap_mod_exp[ 0]
    max_ap = Data.l_ap_mod_exp[-1]

    mu = zfit.Parameter('mu',   4500,   4000,   5000)
    ap = zfit.Parameter('ap', min_ap, min_ap, max_ap)
    bt = zfit.Parameter('bt',  0.002,  0.001,  0.005)
    pdf= ModExp(obs=Data.obs, mu=mu, alpha=ap, beta=bt)

    for ap_val in Data.l_ap_mod_exp:
        ap.set_value(ap_val)
        _plot_pdf(pdf, par_val=ap_val)

    plt.title(r'$\alpha$')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/ap_scan.png')
    plt.close()
# -------------------------------
def test_modexp_bt_scan():
    '''
    Test ModExp PDF scan over beta parameter
    '''
    min_bt = Data.l_bt_mod_exp[ 0]
    max_bt = Data.l_bt_mod_exp[-1]

    mu = zfit.Parameter('mu',   4500,   4000,     5000)
    ap = zfit.Parameter('ap',   0.01,   0.01,     0.02)
    bt = zfit.Parameter('bt', min_bt, min_bt,   max_bt)
    pdf= ModExp(obs=Data.obs, mu=mu, alpha=ap, beta=bt)

    for bt_val in Data.l_bt_mod_exp:
        bt.set_value(bt_val)
        _plot_pdf(pdf, par_val=bt_val)

    plt.title(r'$\beta$')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/bt_scan.png')
    plt.close()
# -------------------------------
