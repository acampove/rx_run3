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

    config_path               = files('post_ap_data').joinpath('tests/post_ap.yaml')
    os.environ['CONFIG_PATH'] = str(config_path)
# ---------------------------------------
def test_dt():
    '''
    Will test filtering of data
    '''
    pfn_path_dt            = files('post_ap_data').joinpath('tests/dt_2024_turbo_comp.json')
    os.environ['PFN_PATH'] = str(pfn_path_dt)

    obj = NtupleFilter(index=0, ngroup=1)
    obj.filter()
# ---------------------------------------
def test_mc():
    '''
    Will test filtering of MC
    '''
    pfn_path_mc            = files('post_ap_data').joinpath('tests/mc_2024_turbo_comp.json')
    os.environ['PFN_PATH'] = str(pfn_path_mc)

    obj = NtupleFilter(index=0, ngroup=1)
    obj.filter()
