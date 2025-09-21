'''
Module with tests for functions in generic/utilities.py
'''
from time    import sleep
from pathlib import Path

import dmu.generic.utilities as gut
import pytest
from omegaconf             import DictConfig, OmegaConf
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test_generic_utilities')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('dmu:generic:utilities'     , 10)    
    LogStore.set_level('dmu:test_generic_utilities', 10)    
# -------------------------
def test_timeit():
    '''
    Will test timer
    '''
    gut.TIMER_ON=True
    @gut.timeit
    def fun():
        sleep(3)

    fun()
# -------------------------
@pytest.mark.parametrize('ext', ['json', 'yaml'])
def test_dump_json(ext : str, tmp_path : Path):
    '''
    Tests dump_json
    '''
    this_path = tmp_path/'dump_json'
    this_path.mkdir(parents=True, exist_ok=True)

    l_data = [1,2,3,4]
    d_data = {
        1           : 'a', 
        'b'         : 2, 
        'some_path' : this_path}

    gut.dump_json(l_data, this_path/f'list.{ext}')
    gut.dump_json(d_data, this_path/f'dict.{ext}')

    oc_l_data = OmegaConf.create(l_data)
    oc_d_data = OmegaConf.create(d_data)

    gut.dump_json(oc_l_data, this_path/f'oc_list.{ext}')
    gut.dump_json(oc_d_data, this_path/f'od_dict.{ext}')

    with pytest.raises(FileExistsError):
        gut.dump_json(oc_l_data, this_path/f'oc_list.{ext}')

    gut.dump_json(oc_l_data, this_path/f'oc_list.{ext}', exists_ok=True)
# -------------------------
@pytest.mark.parametrize('ext', ['json', 'yaml'])
def test_load_json(ext : str, tmp_path : Path):
    '''
    Tests load_json
    '''
    json_dir = tmp_path/'load_json'
    json_dir.mkdir(parents=True, exist_ok=True)
    log.debug(f'Using path: {json_dir}')
    json_path= json_dir/f'list.{ext}'

    l_data_org = [1,2,3,4]
    gut.dump_json(l_data_org, json_path)

    l_data_lod = gut.load_json(json_path)

    assert l_data_org == l_data_lod
# -------------------------
def test_dump_pickle():
    '''
    Tests dump_json and loading it
    '''
    path = '/tmp/tests/dmu/generic/list.pkl'
    data = [1,2,3,4]
    gut.dump_pickle(data=data, path=path)

    loaded = gut.load_pickle(path=path)

    assert data == loaded
# -------------------------
def test_silent_import():
    '''
    Tests decorator in charge of suppressing
    messages from imported modules
    '''
    with gut.silent_import():
        import tensorflow
# -------------------------
@pytest.mark.parametrize('ext', ['yaml', 'json'])
def test_load_data(ext : str):
    '''
    Tests loading file from data package
    '''
    expected = {
            "key" : [
                "value1",
                "value2",
                "value3"]
            }

    data = gut.load_data(
        package='dmu_data',
        fpath  =f'tests/generic/config.{ext}')

    assert data == expected
# -------------------------
def test_load_conf_schema_validation_no_package(no_schema_pkg):
    '''
    Tests that the validation fails when no schema data package is found 
    '''
    cfg = gut.load_conf(
        package='fake_data',
        fpath  ='config.yaml')

    assert cfg == {'dummy' : 'data'}

    with pytest.raises(FileNotFoundError),\
        gut.enforce_schema_validation(value=True):
        cfg = gut.load_conf(
            package='fake_data',
            fpath  ='missing_config.yaml')
# -------------------------
def test_load_conf_schema_validation_no_file():
    '''
    Tests that the validation fails when no schema was found
    '''
    with pytest.raises(FileNotFoundError),\
        gut.enforce_schema_validation(value=True):
        cfg = gut.load_conf(
            package='dmu_data',
            fpath  ='tests/generic/config.yaml')

        assert isinstance(cfg, DictConfig)

        assert cfg.key == ['value1', 'value2', 'value3']
# -------------------------
def test_load_conf_schema_validation():
    '''
    Tests that the validation passes 
    '''
    with gut.enforce_schema_validation(value=True):
        cfg = gut.load_conf(
            package='dmu_data',
            fpath  ='tests/generic/validate.yaml')

        assert isinstance(cfg, DictConfig)

        assert cfg.key == ['value1', 'value2', 'value3']
# -------------------------
def test_load_conf_schema_validation_fail():
    '''
    Tests that the validation fails 
    '''
    with pytest.raises(RuntimeError),\
         gut.enforce_schema_validation(value=True):
        cfg = gut.load_conf(
            package='dmu_data',
            fpath  ='tests/generic/fail_validate.yaml')

        assert isinstance(cfg, DictConfig)

        assert cfg.key == ['value1', 'value2', 'value3']
# -------------------------
@pytest.mark.parametrize('ext', ['yaml', 'json'])
def test_load_conf_format(ext : str):
    '''
    Tests loading configuration from different formats
    '''
    cfg = gut.load_conf(
        package='dmu_data',
        fpath  =f'tests/generic/config.{ext}')

    assert isinstance(cfg, DictConfig)

    assert cfg.key == ['value1', 'value2', 'value3']
# ----------------------
def test_load_conf_reference() -> None:
    '''
    Tests loading of config when fields are paths to sub-configs
    '''
    log.info('')
    name = 'with_references.yaml'

    cfg = gut.load_conf(
        package='dmu_data',
        fpath  =f'tests/generic/{name}')

    assert cfg.config_1.key         == [ 1,   2,   3 ]
    assert cfg.section.config_2.key == ['a', 'b', 'c']

    log.debug(OmegaConf.to_yaml(cfg))
# -------------------------
