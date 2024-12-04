'''
This script containts tests for the ntuple_filter class
'''
import os

from importlib.resources   import files
from dmu.logging.log_store import LogStore
import pytest

from post_ap.ntuple_filter import NtupleFilter 

log = LogStore.add_logger('post_ap:test_ntuple_filter')
# ---------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    LogStore.set_level('post_ap:ntuple_filter' , 10)
    LogStore.set_level('post_ap:FilterFile'    , 10)
    LogStore.set_level('post_ap:selector'      , 10)
    LogStore.set_level('dmu:rdataframe:atr_mgr', 30)

    config_path               = files('post_ap_data').joinpath('v1.yaml')
    os.environ['CONFIG_PATH'] = str(config_path)
# ---------------------------------------
def test_dt():
    '''
    Will test filtering of data
    '''
    obj = NtupleFilter(production='rd_ap_2024', nickname='data', index=1, ngroup=1211)
    obj.filter()
# ---------------------------------------
def test_mc():
    '''
    Will test filtering of MC 
    '''
    obj = NtupleFilter(production='btoxll_mva_2024_nopid', nickname='data', index=1, ngroup=1211)
    obj.filter()
