'''
File containing tests for FilterFile class
'''
import os

import pytest
from dmu.logging.log_store import LogStore
from post_ap.filter_file   import FilterFile

log = LogStore.add_logger('post_ap:test_filter_file')
# --------------------------------------
class Data:
    '''
    Data class with shared attributes
    '''
    mc_turbo1 = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/mc_turbo_001.root'
    mc_turbo2 = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/mc_turbo_002.root'
    mc_spruce = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/mc_spruce.root'

    dt_turbo  = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/dt_turbo.root'
    dt_spruce = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/dt_spruce.root'

    l_args_config = [True, False]
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')
    os.environ['CONFIG_PATH'] = '/home/acampove/Packages/config_files/post_ap/v3_low_stats.yaml'

    LogStore.set_level('dmu:rdataframe:atr_mgr', 30)
    LogStore.set_level('post_ap:selector'      , 20)
    LogStore.set_level('post_ap:utilities'     , 30)
    LogStore.set_level('post_ap:FilterFile'    , 10)
# --------------------------------------
@pytest.mark.parametrize('is_turbo' , [True, False])
def test_dt(is_turbo : bool):
    '''
    Run test on data
    '''
    sample_name = 'data_xxx'
    path        = Data.dt_turbo if is_turbo else Data.dt_spruce

    obj = FilterFile(sample_name=sample_name, file_path=path)
    obj.dump_contents = True
    obj.run(skip_saving=True)
# --------------------------------------
@pytest.mark.parametrize('kind' , ['mc_turbo1', 'mc_turbo2', 'mc_spruce'])
def test_mc(kind : str):
    '''
    Run test on MC
    '''
    sample_name = 'mc_xxx'
    path        = getattr(Data, kind)

    obj = FilterFile(sample_name=sample_name, file_path=path)
    obj.dump_contents = True
    obj.run(skip_saving=False)
# --------------------------------------
