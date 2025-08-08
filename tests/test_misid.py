'''
Module containing tests for MisID class
'''

from dmu.stats.zfit        import zfit
from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut
from fitter.misid          import misid

log=LogStore.add_logger('fitter:test_misid')
# ----------------------
def _validate_pdf(pdf ; zpdf) -> None:
    '''
    Parameters
    -------------
    pdf: KDE to validate
    '''
    pass
# ----------------------
def test_simple() -> None:
    '''
    Basic test for building misID component
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 6000))

    cfg = gut.load_conf(package='fitter_data', fpath='rare/electron/misid.yaml')

    obj = MisID(
            component= 'kkk',
            obs      = obs,
            cfg      = cfg,
            q2bin    = 'central')
    pdf = get_pdf()

    _validate_pdf(pdf=pdf)
