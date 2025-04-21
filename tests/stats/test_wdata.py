'''
Module with functions meant to test the Wdata class
'''
import os
import zfit
import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from dmu.stats.wdata        import Wdata
from dmu.logging.log_store  import LogStore
from dmu.stats.zfit_plotter import ZFitPlotter

log=LogStore.add_logger('dmu:stats:test_wdata')
# --------------------------
class Data:
    '''
    Data class
    '''
    nentries = 1_000
    outdir   = '/tmp/tests/dmu/stats/wdata'
# --------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:stats:wdata', 10)
    os.makedirs(Data.outdir, exist_ok=True)
# --------------------------
def test_create():
    '''
    Tests that the object can be build
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgt  = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)

    data     = Wdata(data=arr_mass, weights=arr_wgt)

    assert isinstance(data, Wdata)
# --------------------------
def test_kde_fit():
    '''
    Tests that KDE fit can be done with data
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgt  = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)
    obs      = zfit.Space('obs', limits=(-3, +3))

    wdata    = Wdata(data=arr_mass, weights=arr_wgt)
    zdata    = wdata.to_zfit(obs=obs)

    pdf      = zfit.pdf.KDE1DimExact(data=zdata, bandwidth='isj')

    obj      = ZFitPlotter(data=zdata, model=pdf)
    obj.plot(nbins=50)
    plt.savefig(f'{Data.outdir}/kde_fit.png')
    plt.close()
# --------------------------
def test_equal():
    '''
    Tests that the object can be build
    '''
    log.info('')
    arr_mass_1 = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgt_1  = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)
    df_1       = pnd.DataFrame({'a' : arr_mass_1, 'b' : arr_mass_1})

    arr_mass_2 = numpy.random.normal(loc=0, scale=1.1, size=Data.nentries)
    arr_wgt_2  = numpy.random.normal(loc=1, scale=0.2, size=Data.nentries)
    df_2       = pnd.DataFrame({'a' : arr_mass_2, 'b' : arr_mass_2})

    wdata_1  = Wdata(data=arr_mass_1, weights=arr_wgt_1, extra_columns=df_1)
    wdata_2  = Wdata(data=arr_mass_1, weights=arr_wgt_1, extra_columns=df_1)
    wdata_3  = Wdata(data=arr_mass_2, weights=arr_wgt_1, extra_columns=None)
    wdata_4  = Wdata(data=arr_mass_1, weights=arr_wgt_2, extra_columns=None)
    wdata_5  = Wdata(data=arr_mass_2, weights=arr_wgt_2, extra_columns=df_2)

    assert wdata_1 == wdata_2
    assert wdata_1 != wdata_3
    assert wdata_1 != wdata_4
    assert wdata_1 != wdata_5
# --------------------------
def test_add():
    '''
    Tests that the object can be build
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgt  = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)
    df       = pnd.DataFrame({'a' : arr_mass, 'b' : arr_mass})
    df_sum   = pnd.concat([df, df], axis=0, ignore_index=True)

    wdata_1  = Wdata(data=arr_mass, weights=arr_wgt, extra_columns=df)
    wdata_2  = Wdata(data=arr_mass, weights=arr_wgt, extra_columns=df)
    wdata_3  = wdata_1 + wdata_2

    assert wdata_1.size + wdata_2.size == wdata_3.size
    assert wdata_1.sumw + wdata_2.sumw == wdata_3.sumw
    assert wdata_3.extra_columns.equals(df_sum)
# --------------------------
def test_update_weights():
    '''
    Tests that the object can be build
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgto = numpy.ones(Data.nentries)
    data_1   = Wdata(data=arr_mass, weights=arr_wgto)

    arr_wgtn = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)
    data_2   = data_1.update_weights(weights=arr_wgtn, replace=False)
    data_3   = data_1.update_weights(weights=arr_wgto, replace= True)

    assert data_1 != data_2
    assert data_1 == data_3
# --------------------------
def test_unweighted():
    '''
    Test with unweighted data
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    data     = Wdata(data=arr_mass)

    assert isinstance(data, Wdata)
# --------------------------
def test_extra_columns():
    '''
    Tests adding extra information on top of the observable and weights
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    df       = pnd.DataFrame({'a' : arr_mass, 'b' : arr_mass})
    data     = Wdata(data=arr_mass, extra_columns=df)

    assert isinstance(data, Wdata)
# --------------------------
def test_repr():
    '''
    Tests the __str__ dunder
    '''
    log.info('')
    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgt  = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)
    df       = pnd.DataFrame({'a' : arr_mass, 'b' : arr_mass})

    data     = Wdata(data=arr_mass, weights=arr_wgt, extra_columns=df)

    log.info(data)
# --------------------------
