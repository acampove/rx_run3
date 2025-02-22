'''
Module with tests for PFNSplitter class
'''
import os
import json
import glob
from importlib.resources   import files
from functools             import cache

import yaml
import pytest
from dmu.logging.log_store import LogStore
from rx_data.path_splitter import PathSplitter

log   = LogStore.add_logger('')
log   = LogStore.add_logger('rx_data:test_path_splitter')
# ----------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir = '/tmp/rx_data/path_splitter'
# ----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:path_splitter', 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------------------
def _save_samples(test : str, data : dict) -> None:
    file_path = f'{Data.out_dir}/{test}.yaml'
    with open(file_path, 'w', encoding='utf-8') as ofile:
        yaml.dump(data, ofile, indent=2)
# ----------------------------------------
@cache
def _get_pfns() -> list[str]:
    jsn_wc = files('rx_data_lfns').joinpath('rx/v4/*.json')
    jsn_wc = str(jsn_wc)
    l_path = glob.glob(jsn_wc)

    l_pfn  = []
    for path in l_path:
        with open(path, encoding='utf-8') as ifile:
            l_pfn += json.load(ifile)

    return l_pfn
# ----------------------------------------
def test_default():
    '''
    Default usage
    '''
    l_pfn = _get_pfns()
    spl   = PathSplitter(paths = l_pfn)
    data  = spl.split()

    _save_samples('default', data)
# ----------------------------------------
def test_max_files():
    '''
    Will only read 100 files
    '''
    l_pfn = _get_pfns()
    spl   = PathSplitter(paths = l_pfn, max_files=100)
    data  = spl.split()

    _save_samples('max_files', data)
# ----------------------------------------
@pytest.mark.parametrize('naming', ['new', 'old'])
def test_sample_naming(naming : str):
    '''
    Will only read 100 files
    '''
    l_pfn = _get_pfns()
    spl   = PathSplitter(paths = l_pfn, sample_naming=naming)
    data  = spl.split()

    _save_samples(f'naming_{naming}', data)
# ----------------------------------------
