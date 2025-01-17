'''
Module with tests for DsGetter class
'''
# pylint: disable=import-error

import pytest
from dmu.logging.log_store  import LogStore

import rx_selection.tests as tst
from rx_selection.ds_getter import DsGetter

log = LogStore.add_logger('rx_selection:test_dsgetter')
# -------------------------------------------
def _get_mva_definitions() -> dict[str,str]:
    d_def               = {}
    d_def['min_ll_pt']  = 'TMath::Min(L1_PT , L2_PT)'
    d_def['max_ll_pt']  = 'TMath::Max(L1_PT , L2_PT)'
    d_def['min_ll_ipc'] = 'TMath::Min(L1_IPCHI2_OWNPV, L2_IPCHI2_OWNPV)'
    d_def['max_ll_ipc'] = 'TMath::Max(L1_IPCHI2_OWNPV, L2_IPCHI2_OWNPV)'

    return d_def
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', tst.get_mc_samples(is_rk=True, included=''))
def test_no_mva(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class without BDT added
    '''

    log.info(f'Running over: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove = ['q2', 'bdt'])
    if cfg is None:
        return

    obj = DsGetter(cfg=cfg)
    _   = obj.get_rdf()
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', tst.get_mc_samples(is_rk=True, included='Bu_Kee_eq_btosllball05_DPC'))
def test_mva(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class with MVA added
    '''

    log.info(f'Running over: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove=[])
    if cfg is None:
        return

    cfg['mva'] = {
            'cmb_low'    : '/home/acampove/Packages/classifier/output/mva_rare_2024_cmb/v2/low',
            'cmb_central': '/home/acampove/Packages/classifier/output/mva_rare_2024_cmb/v2/central',
            'cmb_high'   : '/home/acampove/Packages/classifier/output/mva_rare_2024_cmb/v2/high',
            }

    cfg['Definitions'] = _get_mva_definitions()

    obj = DsGetter(cfg=cfg)
    _   = obj.get_rdf()
# -------------------------------------------
