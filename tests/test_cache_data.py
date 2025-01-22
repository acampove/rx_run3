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
@pytest.mark.parametrize('sample, trigger', tst.get_mc_samples(is_rk=True, included=''))
def test_run3_rk_mc(sample : str, trigger : str):
    '''
    Testing on run3 RK samples and triggers
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = tst.get_config(sample, trigger, is_rk = True, remove=[])
    if cfg is None:
        return

    LogStore.set_level('rx_selection:ds_getter' , 10)
    LogStore.set_level('rx_selection:cache_data', 10)

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
@pytest.mark.parametrize('sample, trigger', tst.get_dt_samples(is_rk=True, included=''))
def test_run3_rk_dt(sample : str, trigger : str):
    '''
    Testing on run3 RK samples and triggers
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = tst.get_config(sample, trigger, is_rk = True, remove=[])
    if cfg is None:
        return

    LogStore.set_level('rx_selection:ds_getter' , 10)
    LogStore.set_level('rx_selection:cache_data', 10)

    # This combination has a very low efficiency, do not limit number of files
    if sample != 'DATA_24_MagDown_24c1' and trigger != 'SpruceRD_BuToHpEE':
        cfg['max_files']  = 10

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
