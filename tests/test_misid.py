'''
Module containing tests for MisID class
'''
import pytest

from dmu.stats.zfit        import zfit
from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut
from zfit.interface        import ZfitPDF   as zpdf
from fitter.misid          import MisID 

log=LogStore.add_logger('fitter:test_misid')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:workflow:cache'      , 30)
    LogStore.set_level('rx_misid:sample_weighter', 20)
# ----------------------
def _validate_pdf(pdf : zpdf) -> None:
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

    cfg = gut.load_conf(package='fitter_data', fpath='misid/electron/data_misid.yaml')

    obj = MisID(
        obs      = obs,
        cfg      = cfg,
        q2bin    = 'central')
    pdf = obj.get_pdf()

    _validate_pdf(pdf=pdf)
# ----------------------
