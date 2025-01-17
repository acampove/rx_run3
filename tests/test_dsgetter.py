'''
Module with tests for DsGetter class
'''
# pylint: disable=import-error

import os
import pytest
from dmu.logging.log_store  import LogStore

import rx_selection.tests as tst
from rx_selection.ds_getter import DsGetter

log = LogStore.add_logger('rx_selection:test_dsgetter')
# -------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_selection:ds_getter', 10)
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
@pytest.mark.parametrize('sample, trigger',
                         tst.get_dt_samples(is_rk=True, included='DATA_24_MagUp_24c2') +
                         tst.get_mc_samples(is_rk=True, included='Bu_Kee_eq_btosllball05_DPC'))
def test_mva(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class with MVA added
    '''

    log.info(f'Running over: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove=['q2', 'bdt'])
    if cfg is None:
        return

    cfg['Definitions'] = _get_mva_definitions()
    cfg['mva']         = {
            'cmb' : {
                'low'    : '/home/acampove/Packages/classifier/output/mva_rare_2024_cmb/v2/low',
                'central': '/home/acampove/Packages/classifier/output/mva_rare_2024_cmb/v2/central',
                'high'   : '/home/acampove/Packages/classifier/output/mva_rare_2024_cmb/v2/high',
                }
            }

    obj = DsGetter(cfg=cfg)
    rdf = obj.get_rdf()

    file_dir  = '/tmp/rx_classifier/ds_getter/mva'
    os.makedirs(file_dir, exist_ok=True)

    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
