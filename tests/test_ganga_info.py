'''
This module has tests for the GangaInfo class
'''
from pathlib            import Path
from rx_data.ganga_info import GangaInfo

def test_ganga_info():
    block = 'w40_42'
    fname = 'mc_magup_11264001_bd_dmnpipl_eq_dpc_Hlt2RD_BuToKpEE_MVA_bd46a6ff81.root'
    dir   = Path('/home/acampove/Tests/filtering')

    with GangaInfo.set_ganga_dir(dir=dir):
        inf = GangaInfo(job_ids=[257])
        assert block == inf.block_from_fname(fname=fname)
