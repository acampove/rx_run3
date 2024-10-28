'''
This script containts tests for the ntuple_filter class
'''

from dmu.logging.log_store import LogStore
import pytest

import data_checks.utilities as ut
from data_checks.ntuple_filter import ntuple_filter

log = LogStore.add_logger('data_checks:test_ntuple_filter')
# ---------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    ut.local_config = True
    LogStore.set_level('data_checks:ntuple_filter', 10)
    LogStore.set_level('data_checks:FilterFile'   , 10)
    LogStore.set_level('data_checks:selector'     , 10)
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
