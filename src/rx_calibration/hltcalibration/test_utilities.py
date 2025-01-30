'''
Module with utility functions needed for tests
'''
from dataclasses              import dataclass

import zfit
from zfit.core.basepdf        import BasePDF
from dmu.stats.model_factory  import ModelFactory
from ROOT                     import RDataFrame, RDF

# --------------------------------------------
@dataclass
class Data:
    '''
    Class sharing attributes
    '''
    out_dir   = '/tmp/rx_calibration/tests/fit_component'
    dat_dir   = '/publicfs/ucas/user/campoverde/Data/RX_run3/for_tests/post_ap'
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
def get_signal_pdf(obs) -> BasePDF:
    '''
    Returns PDFs for signal
    '''
    l_pdf = ['dscb', 'gauss']
    l_shr = ['mu', 'sg']
    mod   = ModelFactory(obs = obs, l_pdf = l_pdf, l_shared=l_shr)
    pdf   = mod.get_pdf()

    return pdf
# --------------------------------------------
def get_toy_pdf(kind : str, obs) -> BasePDF:
    '''
    Makes PDFs
    '''
    if   kind == 'signal':
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
def rdf_from_pdf(pdf : BasePDF, nentries : int) -> RDataFrame:
    '''
    Returns ROOT dataframe from PDF
    '''
    sam     = pdf.create_sampler(n = nentries)
    arr_mas = sam.numpy().flatten()

    return RDF.FromNumpy({'mass' : arr_mas})
# --------------------------------------------
