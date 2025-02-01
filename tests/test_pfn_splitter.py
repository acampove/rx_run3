'''
Module with tests for PFNSplitter class
'''
import json
import glob
from importlib.resources   import files
from functools             import cache

from rx_data.path_splitter import PathSplitter

# ----------------------------------------
@cache
def _get_pfns() -> list[str]:
    jsn_wc = files('rx_data_lfns').joinpath('v4/*.json')
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
    _     = spl.split()
# ----------------------------------------
def test_max_files():
    '''
    Will only read 100 files
    '''
    l_pfn = _get_pfns()
    spl   = PathSplitter(paths = l_pfn, max_files=100)
    _     = spl.split()
# ----------------------------------------
def test_naming():
    '''
    Will only read 100 files
    '''
    l_pfn = _get_pfns()
    spl   = PathSplitter(paths = l_pfn, naming='new')
    _     = spl.split()
# ----------------------------------------
