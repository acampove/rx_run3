'''
Module containing unit tests for Fitter class
'''

import os
from dataclasses import dataclass
from functools   import cache

import numpy
import pytest
import pandas              as pnd
import matplotlib.pyplot   as plt

from dmu.stats              import utilities as sut
from dmu.stats.zfit         import zfit
from dmu.stats.fitter       import Fitter
from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.logging.log_store  import LogStore
from ROOT                   import RDF, gInterpreter

log = LogStore.add_logger('dmu:logging:test_fitter')
#-------------------------------------
@dataclass
class Data:
    '''
    Data class used to store share data
    '''
    nsample = 100_000
    plt_dir = '/tmp/tests/dmu/stats/fitter'

    arr_sig = numpy.random.normal(5.0, 0.5, size=10_000)
    arr_bkg = numpy.random.exponential(scale=10, size=10_000)
    arr_tot = numpy.concatenate((arr_sig, arr_bkg))
    numpy.random.shuffle(arr_tot)

    pdf     = None
    arr     = arr_tot
    obs     = zfit.Space('m', limits=(0, 10))
    df      = pnd.DataFrame({'x' : arr_tot})
    zf      = zfit.data.from_numpy(obs=obs, array=arr)

    l_arg_simple = [arr, df, zf]
#-------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:logging:Fitter', 10)
    os.makedirs(Data.plt_dir, exist_ok=True)
#-------------------------------------
@cache
def _get_weighted_data():
    gInterpreter.ProcessLine('TRandom3 r(1);')

    d_val      = {}
    d_val['x'] = numpy.random.uniform(-1, 4, size=Data.nsample)
    d_val['y'] = numpy.random.uniform(-1, 4, size=Data.nsample)

    rdf = RDF.FromNumpy(d_val)
    rdf = rdf.Define('m', 'r.Gaus(x, 2 + y/4.)')

    return rdf
#-------------------------------------
@cache
def _get_pdf():
    mu  = zfit.Parameter('mu', 5.0,  0, 10)
    sg  = zfit.Parameter('sg', 0.3,  0,  5)
    sig = zfit.pdf.Gauss(obs=Data.obs, mu=mu, sigma=sg)
    nsg = zfit.Parameter('nsg', 100,  0, 10_000_000)
    sig = sig.create_extended(nsg)

    lb  = zfit.Parameter("lb", -0.1,  -0.3, 0)
    bkg = zfit.pdf.Exponential(obs=Data.obs, lam=lb)
    nbg = zfit.Parameter('nbk', 100,  0, 10_000_000)
    bkg = bkg.create_extended(nbg)

    pdf = zfit.pdf.SumPDF([sig, bkg])

    return pdf
#-------------------------------------
def _save_fit(test : str, kind : str):
    out_dir = f'{Data.plt_dir}/{test}'
    os.makedirs(out_dir, exist_ok=True)

    plot_path = f'{out_dir}/{kind}.png'

    log.info(f'Saving to: {plot_path}')
    plt.savefig(plot_path)
    plt.close()
#-------------------------------------
@pytest.mark.parametrize('dat', Data.l_arg_simple)
def test_simple(dat):
    '''
    Simples fitting test
    '''
    pdf = _get_pdf()
    obj = Fitter(pdf, dat)
    res = obj.fit()

    assert res.valid
#-------------------------------------
def test_retry():
    '''
    Test fitting with multiple tries
    '''
    cfg = {'strategy' :
           {'retry' :
            {
                'ntries'        : 10,
                'pvalue_thresh' : 0.05,
                'ignore_status' : False
            }
            }
           }

    pdf = _get_pdf()
    obj = Fitter(pdf, Data.arr)
    res = obj.fit(cfg)

    assert res.valid
#-------------------------------------
def test_constrain():
    '''
    Fits with constraints to parameters
    '''
    cfg = {
            'constraints'   : {
                'mu' : [5.0, 1.0],
                'sg' : [1.0, 0.1],
                }
            }

    pdf = _get_pdf()
    obj=Fitter(pdf, Data.arr)
    res=obj.fit(cfg)

    assert res.valid
#-------------------------------------
def test_ranges():
    '''
    Fit data in disjoint ranges
    '''
    pdf   = sut.get_model('s+b')
    sam   = pdf.create_sampler(n=50_000)
    pdf   = sut.get_model('s+b', lam = -0.3)

    rng   = [[0, 3.5], [6.5, 10]]
    cfg   = {'ranges': rng}
    obj   = Fitter(pdf, sam)
    res   = obj.fit(cfg)

    print(res)
    assert res.valid

    obj   = ZFitPlotter(data=sam, model=pdf)
    obj.plot(nbins=50, stacked=True, ranges=rng)

    _save_fit(test='ranges', kind='fit')
#-------------------------------------
# TODO: Need to improve this test
def test_wgt():
    '''
    Test fit to weighted dataset
    '''
    rdf = _get_weighted_data()
    arr = rdf.AsNumpy(['m'])['m']
    wgt = numpy.random.binomial(1, 0.5, size=arr.size)

    pdf = _get_pdf()
    dat = zfit.data.from_numpy(array=arr, weights=wgt, obs=pdf.space)

    obj=Fitter(pdf, dat)
    res=obj.fit()
#-------------------------------------
def test_steps():
    '''
    Tests the steps fitting strategy
    '''
    pdf = _get_pdf()

    cfg = {
            'strategy' : {
                'steps' : {
                    'nsteps' : [ 1000,  5000],
                    'nsigma' : [  5.0,   2.0],
                    'yields' : ['nsg', 'nbk'],
                    }
                }
            }

    obj = Fitter(pdf, Data.arr)
    res = obj.fit(cfg)
    print(res)

    assert res.valid
#-------------------------------------
@pytest.mark.parametrize('nbins', [None, 100])
def test_binning(nbins : int):
    '''
    Test fitting binnin specified
    '''
    cfg = {'likelihood' : {'nbins' : nbins}}

    pdf = _get_pdf()
    obj = Fitter(pdf, Data.arr)
    res = obj.fit(cfg)

    log.info(res)

    assert res.valid
