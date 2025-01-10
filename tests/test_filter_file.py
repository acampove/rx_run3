'''
File containing tests for FilterFile class
'''
import os
import glob
import shutil
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
    mc_test_turbo    = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/bukmm_turbo.root'
    mc_test_spruce   = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/mc_spruce.root'

    data_test_turbo  = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/dt_turbo.root'
    data_test_spruce = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/dt_spruce.root'

    output_dir = '/home/acampove/cernbox/Run3/post_ap/tests/filter_file'

    l_args_config = [True, False]
# --------------------------------------
def _move_outputs(test_name : str) -> None:
    l_root = glob.glob('*.root')
    l_text = glob.glob('*.txt' )
    l_path = l_root + l_text
    npath  = len(l_path)

    target_dir = f'{Data.output_dir}/{test_name}'
    log.info(f'Moving {npath} to {target_dir}')
    os.makedirs(target_dir, exist_ok=True)
    for source in l_path:
        file_name = os.path.basename(source)
        shutil.move(source, f'{target_dir}/{file_name}')
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    Will set loggers, etc
    '''
    log.info('Initializing')

    cfg_path = files('post_ap_data').joinpath('tests/post_ap.yaml')
    os.environ['CONFIG_PATH'] = str(cfg_path)

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
    sample_name = 'data_test'
    path        = getattr(Data, f'{sample_name}_{kind}')

    obj = FilterFile(sample_name=sample_name, file_path=path)
    obj.dump_contents = True
    obj.run(skip_saving=False)

    _move_outputs('test_dt')
# --------------------------------------
@pytest.mark.parametrize('kind' , ['turbo', 'spruce'])
def test_mc(kind : str):
    '''
    Run test on MC
    '''
    sample_name = 'mc_test'
    path        = getattr(Data, f'{sample_name}_{kind}')

    obj = FilterFile(sample_name=sample_name, file_path=path)
    obj.dump_contents = True
    obj.run(skip_saving=False)

    _move_outputs('test_mc')
# --------------------------------------
