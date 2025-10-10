'''
This module contains unit tests for the validate_slimming utility
'''
import os
from pathlib               import Path
from post_ap_scripts       import validate_slimming as vs
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('post_ap:test_validate_slimming')
# ----------------------
def test_pfn_from_path() -> None:
    '''
    Tests protected _get_pfn_from_path
    '''
    ana_dir  = os.environ['ANADIR']
    fpath    = Path(f'{ana_dir}/Data/rk/main/v11/mc_magdown_11114002_bd_kstmumu_eq_btosllball05_dpc_Hlt2RD_BuToKpMuMu_MVA_7fb0194eee.root')
    path     = vs._get_pfn_from_path(path=fpath)
    expected = 'root://eoslhcb.cern.ch//eos/lhcb/grid/prod/lhcb/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263468/0000/00263468_00000001_1.tuple.root'

    assert path == expected
# ----------------------
def test_get_pfns_from_apd() -> None:
    '''
    Tests _get_pfns_from_apd
    '''
    cfg    = vs.Conf(project='rk', version='v1', path=Path('N/A'))
    s_path = vs._get_pfns_from_apd(cfg=cfg)

    nsam   = len(s_path)
    log.info(f'Found {nsam} samples')

    assert len(s_path) == 1265 
# ----------------------
