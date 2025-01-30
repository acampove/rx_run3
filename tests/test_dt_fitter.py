'''
Module containing tests for Fitter class
'''
# pylint: disable=import-error
# pylint: disable=no-name-in-module

import os
from typing      import Union
from dataclasses import dataclass

import ROOT
import pytest
import numpy
import zfit
import zfit_physics
from ROOT                    import RDataFrame, RDF
from zfit.core.basepdf       import BasePDF
from dmu.logging.log_store   import LogStore

from rx_calibration.hltcalibration.mc_fitter import MCFitter
from rx_calibration.hltcalibration.dt_fitter import DTFitter

log = LogStore.add_logger('rx_calibration:test_dt_fitter')
# --------------------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''
    out_dir    = '/tmp/rx_calibration/tests/fitter'
    mass_name  = 'mass'
    d_nentries = {
            'signal' : 5_000,
            'prec'   : 5_000,
            'comb'   : 5_000,
            }

    l_no_sim   = ['comb']
    obs        = zfit.Space(mass_name, limits=(4000, 6000))
# --------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_calibration:fitter', 10)
# --------------------------------------------
def _rdf_from_pdf(pdf : BasePDF, kind : str) -> Union[RDataFrame,None]:
    out_path = f'{Data.out_dir}/{kind}.root'
    if os.path.isfile(out_path):
        log.warning(f'Reloading: {out_path}')
        rdf = RDataFrame('tree', out_path)
        return rdf

    os.makedirs(Data.out_dir, exist_ok=True)

    samp     = pdf.create_sampler(n=Data.d_nentries[kind])
    arr_mass = samp.numpy().flatten()
    rdf      = RDF.FromNumpy({Data.mass_name : arr_mass})
    rdf.Snapshot('tree', out_path)

    return rdf
# --------------------------------------------
def _get_data_rdf() -> RDataFrame:
    d_rdf   = { component : _get_comp(component)[0] for component in Data.d_nentries }

    l_arr_mass = []
    for rdf in d_rdf.values():
        arr_mass = rdf.AsNumpy([Data.mass_name])[Data.mass_name]
        l_arr_mass.append(arr_mass)

    arr_mass = numpy.concatenate(l_arr_mass)

    return RDF.FromNumpy({Data.mass_name : arr_mass})
# --------------------------------------------
def _get_comp(kind : str) -> tuple[RDataFrame, BasePDF]:
    if   kind == 'signal':
        mu  = zfit.Parameter("mu_flt", 5300, 5200, 5400)
        sg  = zfit.Parameter(    "sg",  40,    30,  100)
        pdf = zfit.pdf.Gauss(obs=Data.obs, mu=mu, sigma=sg)
    elif kind == 'comb':
        lam = zfit.param.Parameter('lam' ,   -1/1000.,  -10/1000.,  0)
        pdf = zfit.pdf.Exponential(obs = Data.obs, lam = lam)
    elif kind == 'prec':
        mzr = zfit.param.Parameter('mzr' ,  10000,  9000, 11000)
        cer = zfit.param.Parameter('cer' ,      2,     0,     3)
        pex = zfit.param.Parameter('pex' ,     10,     8,    15)

        pdf = zfit_physics.pdf.Argus(obs=Data.obs, m0=mzr, c=cer, p=pex)
    else:
        raise ValueError(f'Invalid kind: {kind}')

    rdf = _rdf_from_pdf(pdf, kind)

    return rdf, pdf
# --------------------------------------------
def _get_fit_conf() -> dict:
    return {
            'error_method' : 'minuit_hesse',
            'out_dir'      : '/tmp/rx_calibration/tests/fitter/simple',
            'plotting'     :
            {
                'nbins'   : 50,
                'stacked' : True,
                'd_leg'   : {
                    'Gauss_ext'      : 'Signal',
                    'Exponential_ext': 'Combinatorial',
                    'ArgusPDF_ext'   : 'PRec',
                    }
                },
            }
# --------------------------------------------
def _get_fcomp_cfg(name : str) -> dict:
    d_fcomp = {
            'name'   : name,
            'out_dir': '/tmp/rx_calibration/tests/fitter/components',
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

    if name not in Data.l_no_sim:
        return d_fcomp

    del d_fcomp['fitting' ]
    del d_fcomp['plotting']

    return d_fcomp
# --------------------------------------------
def _get_fit_comp() -> list[MCFitter]:
    d_comp = { component : _get_comp(kind=component) for component in Data.d_nentries }
    for name in Data.l_no_sim:
        _, pdf = d_comp[name]
        d_comp[name] = None, pdf

    l_cfg   = [ _get_fcomp_cfg(component) for component in d_comp          ]
    l_rdf   = [ rdf                       for rdf, _    in d_comp.values() ]
    l_pdf   = [ pdf                       for   _, pdf  in d_comp.values() ]
    l_fcomp = [ MCFitter(cfg=cfg, rdf=rdf, pdf=pdf) for cfg, rdf, pdf in zip(l_cfg, l_rdf, l_pdf)]

    return l_fcomp
# --------------------------------------------
def test_simple():
    '''
    Simplest test of Fitter class
    '''

    rdf_dat = _get_data_rdf()
    l_comp  = _get_fit_comp()
    conf    = _get_fit_conf()

    obj = DTFitter(data = rdf_dat, components = l_comp, conf = conf)
    _   = obj.fit()
# --------------------------------------------
