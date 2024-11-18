'''
File containing tests for FilterFile class
'''

import pytest
from dmu.logging.log_store import LogStore

import post_ap.utilities as ut
from post_ap.filter_file import FilterFile

log = LogStore.add_logger('post_ap:test_filter_file')
# --------------------------------------
class Data:
    '''
    Data class with shared attributes
    '''
    dt_path = '/home/acampove/cernbox/Run3/analysis_productions/Data/mag_down/c2.root'
    mc_path = '/home/acampove/cernbox/Run3/analysis_productions/MC/BdKsmumu.root'

    l_args_config = [True, False]
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')

    LogStore.set_level('rx_scripts:atr_mgr:mgr', 30)
    LogStore.set_level('post_ap:selector'  , 20)
    LogStore.set_level('post_ap:utilities' , 30)
    LogStore.set_level('post_ap:FilterFile', 20)
# --------------------------------------
@pytest.mark.parametrize('local_config', Data.l_args_config)
def test_dt(local_config : bool):
    '''
    Run test on data
    '''
    ut.local_config = local_config

    obj = FilterFile(kind='any_kind', file_path=Data.dt_path, cfg_nam='dt_2024_turbo_comp')
    obj.dump_contents = True
    obj.run()
# --------------------------------------
def test_mc():
    '''
    Run test on MC
    '''
    initialize()

    obj = FilterFile(kind='any_kind', file_path=Data.mc_path, cfg_nam='mc_2024_turbo_comp')
    obj.run()
# --------------------------------------
