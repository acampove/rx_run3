'''
This module has tests for the DataModel class
'''
from dmu.stats.zfit        import zfit
from dmu.generic           import utilities as gut
from dmu.stats             import utilities as sut
from dmu.logging.log_store import LogStore
from fitter.data_model     import DataModel

log=LogStore.add_logger('fitter:test_data_model')
# --------------------------
def test_resonant():
    '''
    Simplest test
    '''

    obs = zfit.Space('B_const_mass_M', limits=(5000, 6000))
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/electron/data.yaml')

    dmd = DataModel(
        cfg     = cfg,
        obs     = obs,
        trigger = 'Hlt2RD_BuToKpEE_MVA',
        project = 'rx',
        q2bin   = 'jpsi',
        name    = 'simple')
    pdf = dmd.get_model()

    sut.print_pdf(pdf)
# --------------------------
