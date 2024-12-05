'''
File containing tests for FilterFile class
'''
import os
from importlib.resources   import files

import pytest
from dmu.logging.log_store import LogStore
from post_ap.filter_file   import FilterFile

log = LogStore.add_logger('post_ap:test_filter_file')
# --------------------------------------
class Data:
    '''
    Data class with shared attributes
    '''
    mc_path = '/home/acampove/cernbox/Run3/analysis_productions/MC/local_tests/mc_2024_w31_34_magup_nu6p3_sim10d_pythia8_12143010_bu_jpsipi_mm_tuple.root'
    dt_path = '/home/acampove/cernbox/Run3/analysis_productions/MC/local_tests/data_24_magdown_turbo_24c3.root'

    l_args_config = [True, False]
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    config_path               = files('post_ap_data').joinpath('v2.yaml')
    os.environ['CONFIG_PATH'] = str(config_path)

    LogStore.set_level('dmu:rdataframe:atr_mgr', 30)
    LogStore.set_level('post_ap:selector'      , 20)
    LogStore.set_level('post_ap:utilities'     , 30)
    LogStore.set_level('post_ap:FilterFile'    , 10)
# --------------------------------------
def test_dt():
    '''
    Run test on data
    '''
    sample_name = 'data_24_magdown_turbo_24c3'

    obj = FilterFile(sample_name=sample_name, file_path=Data.dt_path)
    obj.dump_contents = True
    obj.run()
# --------------------------------------
def test_mc():
    '''
    Run test on MC
    '''
    sample_name = 'mc_2024_w31_34_magup_nu6p3_sim10d_pythia8_12143010_bu_jpsipi_mm_tuple'

    obj = FilterFile(sample_name=sample_name, file_path=Data.mc_path)
    obj.dump_contents = True
    obj.run()
# --------------------------------------
