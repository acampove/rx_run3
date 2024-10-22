'''
Module containing unit tests for Fitter class
'''

import os
from dataclasses import dataclass
from functools   import cache

import ROOT
import zfit
import numpy
import pytest
import pandas              as pnd

from ROOT                  import RDF, gInterpreter
from dmu.stats.fitter      import Fitter
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:logging:test_fitter')
#-------------------------------------
@dataclass
class Data:
    '''
    Data class used to store share data
    '''
    nsample = 50000
    plt_dir = 'tests/Fitter/plots'

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
@pytest.fixture
def _initialize():
    LogStore.set_level('dmu:logging:Fitter', 10)
    os.makedirs(Data.plt_dir, exist_ok=True)
#-------------------------------------
@cache
def _get_data():
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
    obs   = zfit.Space('x', limits=(0, 10))
    lb    = zfit.Parameter("lb", -1,  -2, 0)
    pdf   = zfit.pdf.Exponential(obs=obs, lam=lb)

    data  = numpy.random.exponential(5, size=10000)
    data  = data[(data < 10)]
    data  = data[(data < 2) | ((data > 4) & (data < 6)) |  ((data > 8) & (data < 10)) ]

    cfg   = {'ranges': [(0, 2), (4, 6), (8, 10)]}
    obj   = Fitter(pdf, data)
    res   = obj.fit(cfg)

    assert res.valid
#-------------------------------------
def test_wgt():
    '''
    Test fit to weighted dataset
    '''
    rdf = _get_data()
    arr = rdf.AsNumpy(['m'])['m']
    wgt = numpy.random.binomial(1, 0.5, size=arr.size)

    pdf = _get_pdf()
    dat = zfit.data.from_numpy(array=arr, weights=wgt, obs=pdf.space)

    obj=Fitter(pdf, dat)
    res=obj.fit()

    assert res.valid
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
