'''
This script containts tests for the ntuple_filter class
'''

from dmu.logging.log_store import LogStore
import pytest

import post_ap.utilities as ut
from post_ap.ntuple_filter import ntuple_filter

log = LogStore.add_logger('post_ap:test_ntuple_filter')
# ---------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    ut.local_config = True
    LogStore.set_level('post_ap:ntuple_filter', 10)
    LogStore.set_level('post_ap:FilterFile'   , 10)
    LogStore.set_level('post_ap:selector'     , 10)
    LogStore.set_level('rx_scripts:atr_mgr:mgr'   , 30)
# ---------------------------------------
def test_dt():
    '''
    Will test filtering of data
    '''
    obj = ntuple_filter(dataset='dt_2024_turbo', cfg_ver='comp', index=1, ngroup=1211)
    obj.filter()
# ---------------------------------------
def test_mc():
    '''
    Will test filtering of MC 
    '''
    obj = ntuple_filter(dataset='mc_2024_turbo', cfg_ver='comp', index=1, ngroup=71)
    obj.filter()
