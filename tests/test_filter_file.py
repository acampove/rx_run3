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
    mc_turbo  = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/mc_turbo.root'
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
    LogStore.set_level('post_ap:FilterFile'    , 20)
# --------------------------------------
@pytest.mark.parametrize('kind' , ['turbo', 'spruce'])
def test_dt(kind : bool):
    '''
    Run test on data
    '''
    sample_name = 'dt_'
    path        = getattr(Data, f'{sample_name}{kind}')

    obj = FilterFile(sample_name=sample_name, file_path=path)
    obj.dump_contents = True
    obj.run(skip_saving=False)
# --------------------------------------
@pytest.mark.parametrize('kind' , ['turbo', 'spruce'])
def test_mc(kind : str):
    '''
    Run test on MC
    '''
    sample_name = 'mc_'
    path        = getattr(Data, f'{sample_name}{kind}')

    obj = FilterFile(sample_name=sample_name, file_path=path)
    obj.dump_contents = True
    obj.run(skip_saving=False)
# --------------------------------------
