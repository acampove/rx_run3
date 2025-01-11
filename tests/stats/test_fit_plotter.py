'''
Module with tests for zutils.plot plot class
'''
import os
from dataclasses import dataclass

import zfit
import numpy
import pytest
import matplotlib.pyplot as plt

from zutils.plot  import plot      as zfp
from zutils.utils import split_fit as zfsp

from dmu.logging.log_store import LogStore

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

    obj   = zfp(data=arr, model=pdf, result=None)
    d_leg = {'gauss': 'New Gauss'}
    obj.plot(nbins=50, d_leg=d_leg, plot_range=(-10, 10), ext_text='Extra text here')
    obj.axs[1].set_ylim(-5, 5)

    plt_dir = _make_dir_path(name = 'simple')
    plt.savefig(f'{plt_dir}/fit.png', bbox_inches='tight')
#--------------------------------
