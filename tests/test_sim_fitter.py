'''
This module is meant to test the SimFitter class
'''

from dmu.stats.zfit    import zfit
from dmu.generic       import utilities as gut

from fitter.sim_fitter import SimFitter

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
