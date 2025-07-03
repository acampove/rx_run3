'''
File with tests for PIDWeigher class
'''
import pytest

from dmu.stats.zfit        import zfit
from dmu.logging.log_store import LogStore
from rx_misid.pdf_maker    import PDFMaker

log=LogStore.add_logger('rx_misid:test_pdf_maker')
# ------------------------------------
class Data:
    '''
    Stores shared attributes
    '''
    obs     = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
# ------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter'       , 10)
    LogStore.set_level('rx_misid:misid_calculator', 10)
# ------------------------------------
@pytest.mark.parametrize('sample', ['Bu_KplKplKmn_eq_sqDalitz_DPC'])
def test_simple(sample : str):
    '''
    Simplest test
    '''
    q2bin = 'central'

    mkr = PDFMaker(sample=sample, q2bin=q2bin)
    mkr.get_pdf(obs=Data.obs)
# ------------------------------------
