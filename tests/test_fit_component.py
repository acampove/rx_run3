'''
Module with tests for FitComponent class
'''
# pylint: disable=import-error

import os
from dataclasses                             import dataclass

import pytest
from ROOT                                    import RDataFrame
from zfit.core.basepdf                       import BasePDF
from dmu.logging.log_store                   import LogStore
from rx_calibration.hltcalibration.component import FitComponent

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
# --------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_calibration:fit_component', 10)
# --------------------------------------------
def _get_conf() -> dict:
    return {
            'name'   : 'signal',
            'fitting':
            {
                'error_method'  : 'minuit_hesse',
                'weights_column': 'weights',
                },
            'plotting' :
            {
                'plot_dir': '/tmp/rx_calibration/tests/fitter/simple',
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
def test_simple():
    '''
    Simplest test of FitComponent
    '''
    rdf= _get_rdf('signal')
    pdf= _get_pdf('signal')

    obj=FitComponent(name='signal', rdf=rdf, pdf=pdf)
    pdf=obj.get_pdf(fit=True)
