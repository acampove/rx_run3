'''
Module with tests for CacheData class
'''
# pylint: disable = import-error


import pytest
from dmu.logging.log_store   import LogStore

import rx_selection.tests as tst
from rx_selection.cache_data import CacheData

log = LogStore.add_logger('rx_selection:test_cache_data')

# ---------------------------------------------
def _mc_prc_from_all(l_sample : list[tuple[str,str]]) -> list[tuple[str,str]]:
    l_iso_sample = ['Bd_Kstee_eq_btosllball05_DPC', 'Bu_Kstee_Kpi0_eq_btosllball05_DPC']

    l_sample_mva = [ (sample, trigger) for sample, trigger in l_sample     if 'MVA' in trigger ]
    l_sample_prc = [ (sample, trigger) for sample, trigger in l_sample_mva if sample in l_iso_sample ]

    return l_sample_prc
# ---------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''

    l_mc_sample = tst.get_mc_samples(is_rk=True)
    l_dt_sample = tst.get_dt_samples(is_rk=True)

    l_mc_prc_sample = _mc_prc_from_all(l_mc_sample)
# ---------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_selection:ds_getter' , 10)
    LogStore.set_level('rx_selection:cache_data', 10)
# ---------------------------------------------
@pytest.mark.parametrize('sample, trigger', Data.l_mc_prc_sample)
def test_run3_rk_prc_mc(sample : str, trigger : str):
    '''
    Testing on run3 RK samples used for training of PRec MVA 
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = tst.get_config(sample, trigger, is_rk = True, remove=[])
    if cfg is None:
        return

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
@pytest.mark.parametrize('sample, trigger', Data.l_mc_sample)
def test_run3_rk_all_mc(sample : str, trigger : str):
    '''
    Testing on run3 RK samples and triggers
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = tst.get_config(sample, trigger, is_rk = True, remove=[])
    if cfg is None:
        return

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
@pytest.mark.parametrize('sample, trigger', Data.l_dt_sample)
def test_run3_rk_all_dt(sample : str, trigger : str):
    '''
    Testing on run3 RK samples and triggers
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = tst.get_config(sample, trigger, is_rk = True, remove=[])
    if cfg is None:
        return

    # This combination has a very low efficiency, do not limit number of files
    if sample != 'DATA_24_MagDown_24c1' and trigger != 'SpruceRD_BuToHpEE':
        cfg['max_files']  = 10

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
