'''
Module with tests for FitComponent class
'''
# pylint: disable=import-error

import os
from dataclasses                                 import dataclass

import ROOT
import zfit
import pytest
from ROOT                                        import RDataFrame
from zfit.core.basepdf                           import BasePDF
from dmu.logging.log_store                       import LogStore
from dmu.stats.model_factory                     import ModelFactory
from rx_calibration.hltcalibration.fit_component import FitComponent

log = LogStore.add_logger('rx_calibration:test_fit_component')
# --------------------------------------------
@dataclass
class Data:
    '''
    Class sharing attributes
    '''
    out_dir   = '/tmp/rx_calibration/tests/fit_component'
    nentries  = 5_000
    mass_name = 'mass'
    obs       = zfit.Space(mass_name, limits=(4800, 6000))
# --------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_calibration:fit_component', 10)
    os.makedirs(Data.out_dir, exist_ok=True)
# --------------------------------------------
def _get_conf(name : str) -> dict:
    return {
            'name'   : 'signal',
            'out_dir':f'/tmp/rx_calibration/tests/fit_component/{name}',
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
def _get_rdf(kind : str) -> RDataFrame:
    out_path = f'{Data.out_dir}/{kind}.root'
    if os.path.isfile(out_path):
        log.warning(f'Reloading: {out_path}')
        rdf = RDataFrame('tree', out_path)
        return rdf

    rdf      = RDataFrame(Data.nentries)
    rdf      = rdf.Define(Data.mass_name, 'TRandom3 r(0); return r.Gaus(5300, 50);')

    rdf = rdf.Filter(f'{Data.mass_name} > 4600')

    rdf.Snapshot('tree', out_path)

    return rdf
# --------------------------------------------
def _get_pdf(kind : str) -> BasePDF:
    if   kind == 'signal':
        mu  = zfit.Parameter("mu_flt", 5300, 5200, 5400)
        sg  = zfit.Parameter(    "sg",  10,    10,  100)
        pdf = zfit.pdf.Gauss(obs=Data.obs, mu=mu, sigma=sg)
    elif kind == 'background':
        lam = zfit.param.Parameter('lam' ,   -1/1000.,  -10/1000.,  0)
        pdf = zfit.pdf.Exponential(lam = lam, obs = Data.obs)
    else:
        raise ValueError(f'Invalid kind: {kind}')

    return pdf
# --------------------------------------------
def _get_signal_rdf() -> RDataFrame:
    file_path = '/publicfs/ucas/user/campoverde/Data/RX_run3/v1/post_ap/Bu_JpsiK_ee_eq_DPC/Hlt2RD_BuToKpEE_MVA/mc_magup_12153001_bu_jpsik_ee_eq_dpc_Hlt2RD_BuToKpEE_MVA_c4aa6722b2.root'
    rdf = RDataFrame('DecayTree', file_path)
    rdf = rdf.Define('mass', 'B_const_mass_M')
    rdf = rdf.Range(10_000)

    return rdf
# --------------------------------------------
def _get_signal_pdf() -> BasePDF:
    l_pdf = ['cbr'] + ['cbl']
    l_shr = ['mu', 'sg']
    mod   = ModelFactory(obs = Data.obs, l_pdf = l_pdf, l_shared=l_shr)
    pdf   = mod.get_pdf()

    return pdf
# --------------------------------------------
def test_simple():
    '''
    Simplest test of FitComponent
    '''
    pdf= _get_pdf('signal')
    rdf= _get_rdf('signal')
    cfg= _get_conf('simple')

    obj=FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    pdf=obj.get_pdf()
# --------------------------------------------
def test_bukee():
    '''
    Test using real simulation
    '''
    pdf= _get_signal_pdf()
    rdf= _get_signal_rdf()
    cfg= _get_conf('bukee')

    obj=FitComponent(cfg=cfg, rdf=rdf, pdf=pdf)
    pdf=obj.get_pdf()
# --------------------------------------------

