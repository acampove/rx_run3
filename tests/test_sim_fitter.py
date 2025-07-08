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
