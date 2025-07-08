'''
This module is meant to test the SimFitter class
'''

from dmu.stats.zfit     import zfit
from dmu.generic        import utilities as gut
from rx_data.rdf_getter import RDFGetter

from fitter.sim_fitter  import SimFitter

# ---------------------------------------------------
def test_nomc():
    '''
    Test for components that have no MC associated
    '''
    obs = zfit.Space('B_Mass', limits=(4500, 7000))

    cfg = gut.load_conf(package='fitter_data', fpath='tests/combinatorial.yaml')
    ftr = SimFitter(
        name    = 'combinatorial',
        obs     = obs,
        cfg     = cfg,
        trigger = '',
        project = '',
        q2bin   = '')
    pdf = ftr.get_model()
# ---------------------------------------------------
def test_toy():
    '''
    Simplest test of fitter with toy data
    '''
    obs = zfit.Space('B_Mass', limits=(4500, 7000))

    cfg = gut.load_conf(package='fitter_data', fpath='tests/signal_toy.yaml')
    ftr = SimFitter(
        name    = 'gauss_toy',
        obs     = obs,
        cfg     = cfg,
        trigger = '',
        project = '',
        q2bin   = '')
    pdf = ftr.get_model()
# ---------------------------------------------------
def test_nocat():
    '''
    Test for components without categories, e.g. muon
    '''
    obs = zfit.Space('B_Mass', limits=(5000, 5800))

    cfg = gut.load_conf(package='fitter_data', fpath='tests/signal_muon.yaml')
    with RDFGetter.max_entries(value=10_000):
        ftr = SimFitter(
            name    = 'signal_muon',
            obs     = obs,
            cfg     = cfg,
            trigger = 'Hlt2RD_BuToKpMuMu_MVA',
            project = 'rx',
            q2bin   = 'jpsi')
        pdf = ftr.get_model()
# ---------------------------------------------------
def test_with_cat():
    '''
    Test for components with brem categories 
    '''
    obs = zfit.Space('B_Mass', limits=(4500, 7000))

    cfg = gut.load_conf(package='fitter_data', fpath='tests/signal_electron.yaml')
    with RDFGetter.max_entries(value=100_000):
        ftr = SimFitter(
            name    = 'signal_electron',
            obs     = obs,
            cfg     = cfg,
            trigger = 'Hlt2RD_BuToKpEE_MVA',
            project = 'rx',
            q2bin   = 'jpsi')
        pdf = ftr.get_model()
# ---------------------------------------------------
