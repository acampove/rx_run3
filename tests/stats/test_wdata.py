'''
Module with functions meant to test the Wdata class
'''
import os
import zfit
import numpy
import pytest
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
    log.info(20 * '-')
    log.info('Running test_create')
    log.info(20 * '-')

    arr_mass = numpy.random.normal(loc=0, scale=1.0, size=Data.nentries)
    arr_wgt  = numpy.random.normal(loc=1, scale=0.1, size=Data.nentries)

    data     = Wdata(data=arr_mass, weights=arr_wgt)

    assert isinstance(data, Wdata)
# --------------------------
def test_kde_fit():
    '''
    Tests that KDE fit can be done with data
    '''
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

    arr_mass_2 = numpy.random.normal(loc=0, scale=1.1, size=Data.nentries)
    arr_wgt_2  = numpy.random.normal(loc=1, scale=0.2, size=Data.nentries)

    wdata_1  = Wdata(data=arr_mass_1, weights=arr_wgt_1)
    wdata_2  = Wdata(data=arr_mass_1, weights=arr_wgt_1)
    wdata_3  = Wdata(data=arr_mass_2, weights=arr_wgt_1)
    wdata_4  = Wdata(data=arr_mass_1, weights=arr_wgt_2)
    wdata_5  = Wdata(data=arr_mass_2, weights=arr_wgt_2)

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

    wdata_1  = Wdata(data=arr_mass, weights=arr_wgt)
    wdata_2  = Wdata(data=arr_mass, weights=arr_wgt)
    wdata_3  = wdata_1 + wdata_2

    assert wdata_1.size + wdata_2.size == wdata_3.size
    assert wdata_1.sumw + wdata_2.sumw == wdata_3.sumw
# --------------------------
