'''
Module with utility functions needed for tests
'''
from dataclasses                                 import dataclass

import numpy
import ROOT
import zfit
import zfit_physics

from zfit.core.basepdf                           import BasePDF    as zpdf
from zfit.core.interfaces                        import ZfitSpace  as zobs
from dmu.stats.model_factory                     import ModelFactory
from ROOT                                        import RDataFrame, RDF

from rx_calibration.hltcalibration.fit_component import FitComponent
from rx_calibration.hltcalibration.dt_fitter     import DTFitter
from rx_calibration.hltcalibration.parameter     import Parameter

# --------------------------------------------
@dataclass
class Data:
    '''
    Class sharing attributes
    '''
    out_dir    = '/tmp/rx_calibration/tests'
    dat_dir    = '/publicfs/ucas/user/campoverde/Data/RX_run3/for_tests/post_ap'
    mass_name  = 'mass'
    sign_name  = 'sign'
    d_nentries = {
            sign_name :30_000,
            'prec'    :30_000,
            'comb'    :30_000,
            }

    l_no_sim   = ['comb']
    obs        = zfit.Space(mass_name, limits=(4000, 6000))
# --------------------------------------------
def get_signal_rdf() -> RDataFrame:
    '''
    Returns dataframe with MC
    '''
    file_path = f'{Data.dat_dir}/Bu_JpsiK_ee_eq_DPC/Hlt2RD_BuToKpEE_MVA/mc_magup_12153001_bu_jpsik_ee_eq_dpc_Hlt2RD_BuToKpEE_MVA_c4aa6722b2.root'
    rdf = RDataFrame('DecayTree', file_path)
    rdf = rdf.Define('mass', 'B_const_mass_M')
    rdf = rdf.Range(100_000)

    return rdf
# --------------------------------------------
def _set_pdf_width(pdf : zpdf, width : float) -> None:
    s_par_flt = pdf.get_params(floating= True)
    s_par_fix = pdf.get_params(floating=False)
    s_par     = s_par_flt | s_par_fix
    for par in s_par:
        if not par.name.startswith('sg_'):
            continue

        par.set_value(width)
# --------------------------------------------
def get_signal_pdf(obs : zobs, width : float = 100) -> zpdf:
    '''
    Returns PDFs for signal
    '''
    l_pdf = ['dscb', 'gauss']
    l_shr = ['mu', 'sg']
    mod   = ModelFactory(preffix='signal', obs = obs, l_pdf = l_pdf, l_shared=l_shr, l_float=l_shr)
    pdf   = mod.get_pdf()

    _set_pdf_width(pdf, width)

    return pdf
# --------------------------------------------
def get_toy_pdf(kind : str, obs) -> zpdf:
    '''
    Makes PDFs
    '''
    if   kind == 'sign':
        mu  = zfit.Parameter("mu_flt", 5300, 5200, 5400)
        sg  = zfit.Parameter(    "sg",  30,    10,  100)
        pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)
    elif kind == 'background':
        lam = zfit.param.Parameter('lam' ,   -1/1000.,  -10/1000.,  0)
        pdf = zfit.pdf.Exponential(lam = lam, obs = obs)
    else:
        raise ValueError(f'Invalid kind: {kind}')

    return pdf
# --------------------------------------------
def rdf_from_pdf(pdf : zpdf, nentries : int) -> RDataFrame:
    '''
    Returns ROOT dataframe from PDF
    '''
    sam     = pdf.create_sampler(n = nentries)
    arr_mas = sam.numpy().flatten()

    return RDF.FromNumpy({'mass' : arr_mas})
# --------------------------------------------
def get_data_fit_cfg(test : str) -> dict:
    '''
    Returns configuration for fit to full model
    '''
    return {
            'error_method' : 'minuit_hesse',
            'out_dir'      : f'{Data.out_dir}/{test}',
            'plotting'     :
            {
                'nbins'   : 50,
                'stacked' : True,
                },
            }
# --------------------------------------------
def _get_fit_component_cfg(name : str, test : str) -> dict:
    '''
    Returns configuration for a fit to a given component
    '''
    d_fcomp = {
            'name'   : name,
            'out_dir': f'{Data.out_dir}/{test}/components',
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
def _get_toy_comp(kind : str) -> tuple[RDataFrame, zpdf]:
    if   kind == 'sign':
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

    nentries = Data.d_nentries[kind]
    rdf = rdf_from_pdf(pdf, nentries)

    return rdf, pdf
# --------------------------------------------
def get_fit_components(test : str) -> list[FitComponent]:
    '''
    Function returns list of FitComponent ojects, from toy model
    '''
    d_comp = { component : _get_toy_comp(kind=component) for component in Data.d_nentries }
    for name in Data.l_no_sim:
        _, pdf = d_comp[name]
        d_comp[name] = None, pdf

    l_cfg   = [ _get_fit_component_cfg(component, test) for component     in d_comp                  ]
    l_rdf   = [ rdf                                     for rdf, _        in d_comp.values()         ]
    l_pdf   = [ pdf                                     for   _, pdf      in d_comp.values()         ]
    l_fcomp = [ FitComponent(cfg=cfg, rdf=rdf, pdf=pdf) for cfg, rdf, pdf in zip(l_cfg, l_rdf, l_pdf)]

    return l_fcomp
# --------------------------------------------
def _scale_array(arr_val : numpy.ndarray, eff : float) -> numpy.ndarray:
    l_val = arr_val.tolist()
    nval  = len(l_val) * eff
    nval  = int(nval)
    l_val = l_val[:nval]

    return numpy.array(l_val)
# --------------------------------------------
def get_data_rdf(eff : float = 1.0) -> RDataFrame:
    '''
    Will return dataframe with toy data to fit
    '''
    d_rdf   = { component : _get_toy_comp(component)[0] for component in Data.d_nentries }

    l_arr_mass = []
    for name, rdf in d_rdf.items():
        arr_mass = rdf.AsNumpy([Data.mass_name])[Data.mass_name]
        if name == Data.sign_name:
            arr_mass = _scale_array(arr_mass, eff)

        l_arr_mass.append(arr_mass)

    arr_mass = numpy.concatenate(l_arr_mass)
    arr_mass = numpy.random.permutation(arr_mass)

    if eff == 1.0:
        return RDF.FromNumpy({Data.mass_name : arr_mass})

    return RDF.FromNumpy({Data.mass_name : arr_mass})
# --------------------------------------------
def _get_dt_par(eff : float, name : str) -> Parameter:
    rdf_dat = get_data_rdf(eff)
    l_comp  = get_fit_components(test=name)
    cfg     = get_data_fit_cfg(test=name)

    obj = DTFitter(rdf = rdf_dat, components = l_comp, cfg = cfg)
    par = obj.fit()

    return par
# --------------------------------------------
def get_fit_parameters(eff : float, name : str) -> tuple[Parameter, Parameter]:
    '''
    Simplest test of Fitter class
    '''

    par_pas = _get_dt_par(eff=  eff, name = f'{name}_pass')
    par_fal = _get_dt_par(eff=1-eff, name = f'{name}_fail')

    return par_pas, par_fal
# --------------------------------------------
