'''
Class testing RDFGetter
'''
import pytest
from dmu.logging.log_store  import LogStore
from rx_data.rdf_getter     import RDFGetter

# ------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter', 10)
# ------------------------------------------------
def test_simple():
    '''
    Simplest test of getter class
    '''
    RDFGetter.samples_dir = '/publicfs/ucas/user/campoverde/Data/RX_run3/v4/NO_q2_bdt_mass_Q2_central_VR_v1'

    gtr = RDFGetter(sample='DATA_24_MagUp_24c2', trigger='Hlt2RD_BuToKpEE_MVA')
    _   = gtr.get_rdf()
