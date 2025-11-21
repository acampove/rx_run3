'''
This module has tests for the GangaInfo class
'''
import pytest
from pathlib            import Path

from dmu.generic.utilities import LogStore
from rx_data.ganga_info import GangaInfo

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:ganga_info', 10)
# ----------------------
def test_small():
    '''
    Tests with small inputs
    '''
    block = 'w40_42'
    fname = 'mc_magup_11264001_bd_dmnpipl_eq_dpc_Hlt2RD_BuToKpEE_MVA_bd46a6ff81.root'
    dir   = Path('/home/acampove/Tests/filtering')

    with GangaInfo.set_ganga_dir(dir=dir):
        inf = GangaInfo(job_ids=[257])
        assert block == inf.block_from_fname(fname=fname)
# ----------------------
@pytest.mark.parametrize('job_id', [257, 258])
def test_full(job_id : int):
    '''
    Tests with all inputs
    '''
    inf = GangaInfo(job_ids=[job_id])
