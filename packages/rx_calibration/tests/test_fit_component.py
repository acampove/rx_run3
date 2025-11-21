'''
Module with tests for Fitter class
'''
# pylint: disable=import-error, wrong-import-position

import os
from dataclasses                                 import dataclass

import pytest
import pandas as pnd
from dmu.stats.zfit                              import zfit
from dmu.logging.log_store                       import LogStore
from ROOT                                        import RDF
from rx_calibration.hltcalibration.fit_component import FitComponent, load_fit_component
from rx_calibration.hltcalibration               import test_utilities as tut

log = LogStore.add_logger('rx_calibration:test_mc_fitter')
# --------------------------------------------
@dataclass
class Data:
    '''
    Class sharing attributes
    '''
    out_dir   = '/tmp/rx_calibration/tests/fit_component'
    dat_dir   = '/publicfs/ucas/user/campoverde/Data/RX_run3/for_tests/post_ap'
    nentries  = 5_000
    mass_name = 'mass'
    obs       = zfit.Space(mass_name, limits=(4500, 6000))
# --------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_calibration:fit_component', 10)
    os.makedirs(Data.out_dir, exist_ok=True)
# --------------------------------------------
def _get_conf(name : str) -> dict:
    return {
            'name'   : name,
            'output' : {'out_dir': f'/tmp/tests/rx_calibration/fit_component/{name}'},
            'fitting':
            {
                'error_method'  : 'minuit_hesse',
                'weights_column': 'weights',
                },
            'plotting' :
            {
                'nbins'   : 50,
                'stacked' : True,
                },
            }
# --------------------------------------------
def test_toy_pdf():
    '''
    Simplest test of MCFitter
    '''
    log.info('')

    pdf= tut.get_toy_pdf(kind='sign', obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    cfg= _get_conf('toy')

    obj=FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    _  =obj.run()
# --------------------------------------------
@pytest.mark.parametrize('width', [30, 50, 100, 150, 200])
def test_kde_pdf(width : float):
    '''
    Test using KDE
    '''
    name = f'kde_width_{width:03}'

    pdf= tut.get_signal_pdf(obs=Data.obs, width=width)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    cfg= _get_conf(name)
    cfg['fitting'] = {
            'config' : {
                name : {
                    'cfg_kde':
                    {
                        'padding'  : {'lowermirror': 1.0, 'uppermirror': 1.0},
                        },
                    }
                }
            }

    obj= FitComponent(cfg=cfg, rdf=rdf, pdf=None, obs=Data.obs)
    _  = obj.run()
# --------------------------------------------
def test_no_data():
    '''
    Simplest test of MCFitter
    '''
    log.info('')

    pdf= tut.get_toy_pdf(kind='sign', obs=Data.obs)
    cfg= _get_conf('no_data')

    obj=FitComponent(cfg=cfg, rdf=None, pdf=pdf)
    _  =obj.run()
# --------------------------------------------
def test_load_pdf_fail():
    '''
    Tests that if PDF does not exist, loader returns None
    '''
    pdf= tut.get_toy_pdf(kind='sign', obs=Data.obs)
    cfg= _get_conf('load_pdf_fail')

    obj=load_fit_component(cfg=cfg, pdf=pdf)

    assert obj is None
# --------------------------------------------
def test_load_pdf_work():
    '''
    Tests loading PDF when it already exists
    '''
    pdf= tut.get_toy_pdf(kind='sign', obs=Data.obs)
    cfg= _get_conf('load_pdf_work')

    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    obj=FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    _  =obj.run()

    obj=load_fit_component(cfg=cfg, pdf=pdf)

    assert obj is not None
# --------------------------------------------
def test_kde_empty_pdf():
    '''
    Test using KDE with empty dataset
    '''
    name = 'kde_empty'

    pdf= tut.get_signal_pdf(obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    rdf= rdf.Filter('mass != mass')

    cfg= _get_conf(name)
    cfg['fitting'] = {
            'config' : {
                name : {
                    'cfg_kde':
                    {
                        'padding'  : {'lowermirror': 0.5, 'uppermirror': 0.5},
                        },
                    }
                }
            }

    obj= FitComponent(cfg=cfg, rdf=rdf, pdf=None, obs=Data.obs)
    _  = obj.run()
# --------------------------------------------
@pytest.mark.parametrize('nentries', [50, 100, 200, 400, 600])
def test_kde_stats(nentries : int):
    '''
    Test using KDE with different dataset sizes 
    '''
    name = f'kde_stats_{nentries:03}'

    pdf= tut.get_signal_pdf(obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=nentries)

    cfg= _get_conf(name)
    cfg['fitting'] = {
            'config' : {
                name : {
                    'cfg_kde':
                    {
                        'padding'  : {'lowermirror': 0.5, 'uppermirror': 0.5},
                        },
                    }
                }
            }

    obj= FitComponent(cfg=cfg, rdf=rdf, pdf=None, obs=Data.obs)
    _  = obj.run()
# --------------------------------------------
def test_pdf():
    '''
    Test retrieving PDF only
    '''
    name = 'kde_empty'

    pdf= tut.get_signal_pdf(obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    rdf= rdf.Filter('mass != mass')

    cfg= _get_conf(name)
    cfg['fitting'] = {
            'config' : {
                name : {
                    'cfg_kde':
                    {
                        'padding'  : {'lowermirror': 0.5, 'uppermirror': 0.5},
                        },
                    }
                }
            }

    obj= FitComponent(cfg=cfg, rdf=rdf, pdf=None, obs=Data.obs)
    pdf= obj.get_pdf()
# --------------------------------------------
def test_kde_cache_pdf():
    '''
    Test using KDE
    '''
    name = 'kde_cache'

    pdf= tut.get_signal_pdf(obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    cfg= _get_conf(name)
    cfg['fitting'] = {
            'config' : {
                name : {
                    'cfg_kde':
                    {
                        'padding'  : {'lowermirror': 0.5, 'uppermirror': 0.5},
                        },
                    }
                }
            }

    obj       = FitComponent(cfg=cfg, rdf=rdf, pdf=None, obs=Data.obs)
    pdf_orig  = obj.get_pdf()

    out_dir   = cfg['output']['out_dir']
    json_path = f'{out_dir}/data.json'
    df        = pnd.read_json(json_path)
    rdf       = RDF.FromPandas(df)

    obj       = FitComponent(cfg=cfg, rdf=rdf, pdf=None, obs=Data.obs)
    pdf_cached= obj.get_pdf()
# --------------------------------------------
def test_custom_title():
    '''
    Tests adding custom title
    '''
    log.info('')

    pdf= tut.get_toy_pdf(kind='sign', obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    cfg= _get_conf('custom_title')
    cfg['plotting']['title'] = 'some title'

    obj=FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    _  =obj.run()
# --------------------------------------------
