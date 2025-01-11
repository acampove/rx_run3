'''
Module with tests for zutils.plot plot class
'''
import os
from dataclasses import dataclass

import zfit
import numpy
import pytest
import matplotlib.pyplot as plt

from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('dmu:test_fit_plotter')
#--------------------------------
@dataclass
class Data:
    '''
    Class storing shared variables
    '''
    l_arg_xerror = [
            (True , 'xerr_on' ),
            (False, 'xerr_off'),
            (0.5  , 'xerr_0p5'),
            ]
#--------------------------------
def _make_dir_path(name : str) -> str:
    path = f'/tmp/dmu/tests/fit_plotter/{name}'
    os.makedirs(path, exist_ok=True)

    return path
#--------------------------------
def test_simple():
    '''
    Simplest test of plotting class
    '''
    arr = numpy.random.normal(0, 1, size=1000)

    obs = zfit.Space('m', limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.0, -5, 5)
    sg  = zfit.Parameter("sg", 1.0,  0, 5)

    pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg, name='gauss')
    nev = zfit.Parameter('nev', 1000, 0, 10000)
    pdf = pdf.create_extended(nev,)

    obj   = ZFitPlotter(data=arr, model=pdf, result=None)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(nbins=50, d_leg=d_leg, plot_range=(-10, 10), ext_text='Extra text here')
    obj.axs[1].set_ylim(-5, 5)

    plt_dir = _make_dir_path(name = 'simple')
    plt.savefig(f'{plt_dir}/fit.png', bbox_inches='tight')
#--------------------------------
def test_title():
    '''
    Testing adding title in plot
    '''
    arr = numpy.random.normal(0, 1, size=1000)

    obs = zfit.Space('m', limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.0, -5, 5)
    sg  = zfit.Parameter("sg", 1.0,  0, 5)

    pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg, name='gauss')
    nev = zfit.Parameter('nev', 1000, 0, 10000)
    pdf = pdf.create_extended(nev,)

    obj   = ZFitPlotter(data=arr, model=pdf, result=None)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(title='title here', nbins=50, d_leg=d_leg, plot_range=(-10, 10), ext_text='Extra text here')
    obj.axs[1].set_ylim(-5, 5)

    plt_dir = _make_dir_path(name = 'title')
    plt.savefig(f'{plt_dir}/fit.png', bbox_inches='tight')
#--------------------------------
def test_legend():
    '''
    Tests overriding of PDF name in legend
    '''
    obs = zfit.Space('m', limits=(0, 10))

    mu  = zfit.Parameter("mu", 5.0,  0, 10)
    sg  = zfit.Parameter("sg", 0.5,  0,  5)
    sig = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)
    nsg = zfit.Parameter('nsg', 1000, 0, 10000)
    esig= sig.create_extended(nsg, name='gauss')

    lm  = zfit.Parameter('lm', -0.1, -1, 0)
    bkg = zfit.pdf.Exponential(obs=obs, lam=lm)
    nbk = zfit.Parameter('nbk', 1000, 0, 10000)
    ebkg= bkg.create_extended(nbk, name='expo')

    pdf = zfit.pdf.SumPDF([ebkg, esig])
    sam = pdf.create_sampler()

    obj   = ZFitPlotter(data=sam, model=pdf)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(nbins=50, d_leg=d_leg, stacked=False, plot_range=(0, 10), ext_text='Extra text here')

    # add a line to pull hist
    obj.axs[1].plot([0, 10], [0, 0], linestyle='--', color='black')

    plt_dir = _make_dir_path(name = 'legend')
    plt.savefig(f'{plt_dir}/fit_lin.png', bbox_inches='tight')

    obj.axs[0].set_yscale('log')
    plt.savefig(f'{plt_dir}/fit_log.png', bbox_inches='tight')
#--------------------------------
def test_size():
    '''
    Tests use of figsize argument
    '''
    arr = numpy.random.normal(0, 1, size=1000)

    obs = zfit.Space('m', limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.0, -5, 5)
    sg  = zfit.Parameter("sg", 1.0,  0, 5)

    pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg, name='gauss')
    nev = zfit.Parameter('nev', 1000, 0, 10000)
    pdf = pdf.create_extended(nev,)

    obj   = ZFitPlotter(data=arr, model=pdf, result=None)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(nbins=50, d_leg=d_leg, plot_range=(-10, 10), ext_text='Extra text here', figsize=(30, 10))
    obj.axs[1].set_ylim(-5, 5)

    plt_dir = _make_dir_path(name = 'size')
    plt.savefig(f'{plt_dir}/fit.png', bbox_inches='tight')
#--------------------------------
def test_leg_loc():
    '''
    Test usage of leg_loc argument, to place legend
    '''
    arr = numpy.random.normal(0, 1, size=1000)

    obs = zfit.Space('m', limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.0, -5, 5)
    sg  = zfit.Parameter("sg", 1.0,  0, 5)

    pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg, name='gauss')
    nev = zfit.Parameter('nev', 1000, 0, 10000)
    pdf = pdf.create_extended(nev,)

    obj   = ZFitPlotter(data=arr, model=pdf, result=None)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(nbins=50, d_leg=d_leg, plot_range=(-10, 10), ext_text='Extra text here', leg_loc='upper left')
    obj.axs[1].set_ylim(-5, 5)

    plt_dir = _make_dir_path(name = 'leg_loc')
    plt.savefig(f'{plt_dir}/fit.png', bbox_inches='tight')
#--------------------------------
def test_weights():
    '''
    Tests plotting of weighted zfit data
    '''
    obs = zfit.Space('m', limits=(0, 10))
    mu  = zfit.Parameter("mu",    5,  4.0, 6)
    sg1 = zfit.Parameter("sg1", 1.0,  0.5, 2)
    sg2 = zfit.Parameter("sg2", 2.0,  0.5, 2)

    mu.floating = False
    sg1.floating= False
    sg2.floating= False

    pdf_a = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg1, name='gauss_1')
    pdf_b = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg2, name='gauss_2')

    nev_1 = zfit.Parameter('nev_1', 200, 1, 100000)
    nev_2 = zfit.Parameter('nev_2', 300, 1, 100000)

    pdf_1 = pdf_a.create_extended(nev_1, name='G1')
    pdf_2 = pdf_b.create_extended(nev_2, name='G2')
    pdf   = zfit.pdf.SumPDF([pdf_1, pdf_2], name='Model')

    arr   = numpy.random.normal(loc=5, scale=1, size=1000)
    wgt   = numpy.random.binomial(1, 0.5, size=arr.size)
    dat   = zfit.Data.from_numpy(obs=obs, array=arr, weights=wgt)

    nll = zfit.loss.ExtendedUnbinnedNLL(model=pdf, data=dat)
    mnm = zfit.minimize.Minuit()
    res = mnm.minimize(nll)

    #Fake GOF for ploting purposes
    res.gof = (11, 10, 0.5)

    obj   = ZFitPlotter(data=dat, model=pdf, result=res)
    obj.plot(nbins=50)

    # add a line to pull hist
    [[lower]], [[upper]] = obs.limits
    obj.axs[1].plot([lower, upper], [0, 0], linestyle='--', color='black')

    plt_dir = _make_dir_path(name = 'weights')
    plt.savefig(f'{plt_dir}/fit_lin.png', bbox_inches='tight')
#--------------------------------
def test_low_stat():
    '''
    Testing fit plots with low statistics data
    '''
    arr = numpy.random.normal(0, 1, size=10)

    obs = zfit.Space('m', limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.4, -5, 5)
    sg  = zfit.Parameter("sg", 1.3,  0, 5)

    pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg, name='gauss')
    nev = zfit.Parameter('nev', 100, 0, 10000)
    pdf = pdf.create_extended(nev,)

    dat = zfit.Data.from_numpy(obs=obs, array=arr)
    nll = zfit.loss.ExtendedUnbinnedNLL(model=pdf, data=dat)
    mnm = zfit.minimize.Minuit()
    res = mnm.minimize(nll)

    #Fake GOF for ploting purposes
    res.gof = (11, 10, 0.5)

    obj   = ZFitPlotter(data=arr, model=pdf, result=res)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(nbins=50, d_leg=d_leg, plot_range=(0, 10), ext_text='Extra text here')

    # add a line to pull hist
    lower, upper = dat.data_range.limit1d
    obj.axs[1].plot([lower, upper], [0, 0], linestyle='--', color='black')

    plt_dir = _make_dir_path(name = 'low_stat')
    plt.savefig(f'{plt_dir}/fit_lin.png', bbox_inches='tight')

    obj.axs[0].set_yscale('log')
    plt.savefig(f'{plt_dir}/fit_log.png', bbox_inches='tight')
#--------------------------------
