'''
Module with tests for Fitter class
'''
# pylint: disable=import-error

import os
from dataclasses                                 import dataclass

import ROOT
import zfit
import pytest
from dmu.logging.log_store                       import LogStore
from rx_calibration.hltcalibration.fit_component import FitComponent
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
    obs       = zfit.Space(mass_name, limits=(5200, 5400))
# --------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_calibration:fit_component', 10)
    os.makedirs(Data.out_dir, exist_ok=True)
# --------------------------------------------
def _get_conf(name : str) -> dict:
    return {
            'name'   : name,
            'out_dir': f'/tmp/rx_calibration/tests/fit_component/{name}',
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
    pdf= tut.get_toy_pdf(kind='sign', obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    cfg= _get_conf('toy')

    obj=FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    _  =obj.run()
# --------------------------------------------
def test_real_pdf():
    '''
    Test using real simulation
    '''
    pdf= tut.get_signal_pdf(obs=Data.obs)
    rdf= tut.rdf_from_pdf(pdf=pdf, nentries=Data.nentries)
    cfg= _get_conf('bukee')

    obj= FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    _  = obj.run()
# --------------------------------------------
