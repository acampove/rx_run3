'''
Unit test for plotter class in dmu.plotting
'''
#pylint: disable=no-name-in-module

from typing              import Union
from importlib.resources import files
from dataclasses         import dataclass

import pytest
import matplotlib.pyplot as plt
import yaml
import numpy
import mplhep

from ROOT                    import RDF, RDataFrame
from dmu.plotting.plotter_1d import Plotter1D as Plotter
from dmu.logging.log_store   import LogStore
#---------------------------------------
@dataclass
class Data:
    '''
    Class used to store shared data
    '''
#---------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    This needs to be ran before any test
    '''
    plt.style.use(mplhep.style.LHCb2)

    LogStore.set_level('dmu:plotting:Plotter'  , 10)
    LogStore.set_level('dmu:plotting:Plotter1D', 10)
#---------------------------------------
def _get_rdf(kind : str, test : str, nentries : Union[int,None] = None) -> RDataFrame:
    '''
    kind (str): "class A" or "class B", equivalent to data or mc, but general
    test (str): Identifies specific test
    '''
    if nentries is not None:
        pass
    elif test == 'high_stat':
        nentries = 1_000_000
    else:
        nentries =   100_000

    d_data = {}
    if   kind == 'class A':
        d_data['x'] = numpy.random.normal(0, 1, size=nentries)
        d_data['y'] = numpy.random.normal(0, 2, size=nentries)
    elif kind == 'class B':
        d_data['x'] = numpy.random.normal(1, 1, size=nentries)
        d_data['y'] = numpy.random.normal(1, 2, size=nentries)
    else:
        raise ValueError(f'Invalid class: {kind}')

    d_data['weights'] = numpy.random.normal(0.5, 0.1, size=nentries)
    rdf = RDF.FromNumpy(d_data)

    return rdf
#---------------------------------------
def _load_config(test : str):
    '''
    test (str): Identifies specific test
    '''

    cfg_path = files('dmu_data').joinpath(f'plotting/tests/{test}.yaml')
    cfg_path = str(cfg_path)
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    plt_dir = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir'] = f'/tmp/dmu/{plt_dir}'

    return cfg
#---------------------------------------
def test_simple():
    '''
    Minimal test
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='simple')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_styling():
    '''
    Change style of histogram plots
    '''
    d_rdf   =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }
    cfg_dat = _load_config(test='styling')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_stats():
    '''
    Test for addition of statistics
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='stats', nentries=12345) for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='stats')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_high_stat():
    '''
    Test for large datasets
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='high_stat') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='high_stat')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_no_bounds():
    '''
    Test for case where plot bounds are not explicitly passed
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='no_bounds')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_fig_size():
    '''
    Test for fig size setting
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='fig_size')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_title():
    '''
    Test for title
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='title')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_weights():
    '''
    Tests plotting with weights
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='weights') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='weights')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_name():
    '''
    Testing the use of the name key, to name PNG files
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='name')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_normalized():
    '''
    Will test the plot of normalized histograms
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='normalized')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_legend():
    '''
    Tests legend options
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='legend')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def test_plugin_fwhm():
    '''
    Will test fwhm plugin
    '''
    d_rdf =  { kind : _get_rdf(kind=kind, test='simple') for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(test='simple')
    cfg_dat['saving']['plt_dir'] = '/tmp/dmu/plugin_fwhm'
    cfg_dat['plugin'] = {'fwhm' : {'plot' : True, 'format' : r'FWHM={:.3f}', 'obs' : [-2, 3], 'add_std' : True}}

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
