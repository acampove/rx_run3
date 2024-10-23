'''
This script containts tests for the ntuple_filter class
'''

from log_store                 import log_store

import data_checks.utilities as ut
from data_checks.ntuple_filter import ntuple_filter

log = log_store.add_logger('data_checks:test_ntuple_filter')
# ---------------------------------------
def initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    ut.local_config = True
    log_store.set_level('data_checks:ntuple_filter', 10)
    log_store.set_level('data_checks:FilterFile'   , 10)
    log_store.set_level('data_checks:selector'     , 10)
    log_store.set_level('rx_scripts:atr_mgr:mgr'   , 30)
# ---------------------------------------
def test_dt():
    '''
    Will test filtering of data
    '''
    initialize()

    obj = ntuple_filter(dataset='dt_2024_turbo', cfg_ver='comp', index=1, ngroup=1211)
    obj.filter()
# ---------------------------------------
def test_mc():
    '''
    Will test filtering of MC 
    '''
    obj = ntuple_filter(dataset='mc_2024_turbo', cfg_ver='comp', index=1, ngroup=71)
    obj.filter()
