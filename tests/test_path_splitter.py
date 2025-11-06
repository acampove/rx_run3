'''
Module with tests for PFNSplitter class
'''
import os
import json
import glob
from pathlib               import Path
from importlib.resources   import files
from functools             import cache

import yaml
import pytest
from dmu.logging.log_store import LogStore
from dmu.generic           import version_management as vmn
from rx_data.path_splitter import PathSplitter

log   = LogStore.add_logger('')
log   = LogStore.add_logger('rx_data:test_path_splitter')
# ----------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir = '/tmp/tests/rx_data/path_splitter'
# ----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:path_splitter', 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------------------
def _save_samples(test : str, data : dict) -> None:
    file_path = f'{Data.out_dir}/{test}.yaml'
    with open(file_path, 'w', encoding='utf-8') as ofile:
        yaml.dump(data, ofile, indent=2)
# ----------------------------------------
@cache
def _get_pfns(analysis : str) -> list[Path]:
    lfn_dir= files('rx_data_lfns').joinpath(analysis)
    lfn_dir= Path(str(lfn_dir))
    lfn_ver= vmn.get_last_version(dir_path=lfn_dir, version_only=False)
    l_path = lfn_ver.glob(pattern='*.json')

    l_pfn  = []
    for path in l_path:
        with open(path, encoding='utf-8') as ifile:
            l_pfn += json.load(ifile)

    return l_pfn
# ----------------------------------------
@pytest.mark.parametrize('analysis', ['rx', 'nopid'])
def test_default(analysis : str):
    '''
    Default usage
    '''
    pfns  = _get_pfns(analysis=analysis)
    spl   = PathSplitter(paths = pfns)
    data  = spl.split()

    _save_samples(f'default_{analysis}', data)
# ----------------------------------------
@pytest.mark.parametrize('analysis', ['rx', 'nopid'])
def test_nested(analysis : str):
    '''
    Dumps it with nesting sample:trigger:list of files
    '''
    l_pfn = _get_pfns(analysis=analysis)
    spl   = PathSplitter(paths = l_pfn)
    data  = spl.split(nested=True)

    _save_samples(f'nested_{analysis}', data)
# ----------------------------------------
@pytest.mark.parametrize('analysis', ['rx', 'nopid'])
def test_max_files(analysis : str):
    '''
    Will only read 100 files
    '''
    l_pfn = _get_pfns(analysis=analysis)
    spl   = PathSplitter(paths = l_pfn, max_files=100)
    data  = spl.split()

    _save_samples(f'max_files_{analysis}', data)
# ----------------------------------------
@pytest.mark.parametrize('analysis', ['rx', 'nopid'])
@pytest.mark.parametrize('naming'  , ['new', 'old'])
def test_sample_naming(naming : str, analysis : str):
    '''
    Will only read 100 files
    '''
    l_pfn = _get_pfns(analysis=analysis)
    spl   = PathSplitter(paths = l_pfn, sample_naming=naming)
    data  = spl.split()

    _save_samples(f'naming_{naming}_{analysis}', data)
# ----------------------------------------
