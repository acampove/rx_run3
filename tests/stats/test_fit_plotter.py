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
