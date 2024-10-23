'''
File containing tests for FilterFile class
'''

from log_store               import log_store


import data_checks.utilities as ut
from data_checks.filter_file import FilterFile

log = log_store.add_logger('data_checks:test_filter_file')
# --------------------------------------
class Data:
    '''
    Data class with shared attributes
    '''
    dt_path = '/home/acampove/cernbox/Run3/analysis_productions/Data/mag_down/c2.root'
    mc_path = '/home/acampove/cernbox/Run3/analysis_productions/MC/BdKsmumu.root'
# --------------------------------------
def initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    ut.local_config = True

    log_store.set_level('rx_scripts:atr_mgr:mgr', 30)
    log_store.set_level('data_checks:selector'  , 20)
    log_store.set_level('data_checks:utilities' , 30)
    log_store.set_level('data_checks:FilterFile', 20)
# --------------------------------------
def test_dt():
    '''
    Run test on data
    '''
    initialize()

    obj = FilterFile(kind='any_kind', file_path=Data.dt_path, cfg_nam='dt_2024_turbo_comp')
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
