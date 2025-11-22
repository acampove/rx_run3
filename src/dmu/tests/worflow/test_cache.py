'''
Module used to test Cache class
'''
import os
import pytest

from pathlib               import Path
from dmu.generic           import utilities as gut
from dmu.workflow.cache    import Cache     as Wcache
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:workflow:test_cache')
# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    LogStore.set_level('dmu:workflow:cache', 10)
# -----------------------------------
class Tester(Wcache):
    '''
    Testing class, produces outputs from simple inputs
    '''
    # -----------------------------------
    def __init__(
        self,
        name   : str,
        nval   : int,
        val    : int  = 1,
        add_dir: bool = False):
        '''
        name   : Identifies instance of Tester
        nval   : Some integer used to produce output data
        add_dir: If true, will produce output with directory
        '''
        super().__init__(
            out_path=name,
            val     =val,
            nval    =nval)

        self._nval    = nval
        self._val     = val
        self._add_dir = add_dir
    # -----------------------------------
    def run(self) -> list[int]:
        '''
        Returns a list of 1's
        '''
        obj_path = f'{self._out_path}/values.json'

        if self._copy_from_cache():
            val = gut.load_json(obj_path)
            log.warning(f'Output cached, returning: {val}')

            return val

        res = [self._val] * self._nval
        log.info(f'Data not cached, saving: {res}')

        empty_dir_path = f'{self._out_path}/some_empty_dir'
        full_dir_path  = f'{self._out_path}/some_dir'

        os.makedirs(empty_dir_path)
        os.makedirs(full_dir_path)
        with open(f'{full_dir_path}/file.txt', 'w', encoding='utf-8') as ofile:
            ofile.write('xxx')

        gut.dump_json(res, obj_path)
        self._cache()

        return res
# -----------------------------------
@pytest.mark.parametrize('nval', range(1, 11))
def test_cache_parallel(nval : int):
    '''
    Tests running caching in parallel
    '''
    val = 1

    log.info('')
    res = nval * [val]
    obj = Tester(nval=nval, val=val, name=f'worker_{nval:03}')
    out = obj.run()

    assert res == out
# -----------------------------------
def test_cache_once():
    '''
    Will run once
    '''
    log.info('')
    res = 4 * [1]
    obj = Tester(nval=4, name='cache_once')
    out = obj.run()

    assert res == out
# -----------------------------------
def test_cache():
    '''
    Tests that value is the correct one when using same inputs
    '''
    log.info('')
    res = 4 * [1]
    for _ in range(2):
        obj = Tester(nval=4, name='cache')
        out = obj.run()

        assert res == out
# -----------------------------------
def test_update():
    '''
    Tests case where inputs change
    '''
    for val in range(10):
        log.info(f'Testing with: {val}')
        obj = Tester(nval=val, name='update')
        out = obj.run()

        assert out == [1] * val
        log.info('')
# -----------------------------------
# TODO: Improve this test
def test_dont_cache():
    '''
    Tests running with caching turned off
    '''
    log.info('')
    res = 4 * [1]
    for _ in range(2):
        with Wcache.turn_off_cache(val=['Tester']):
            obj = Tester(nval=4, name='dont_cache')
            out = obj.run()

        assert res == out
# -----------------------------------
def test_cache_with_dir():
    '''
    Will cache where the outputs contain a directory
    '''
    log.info('')
    res = 4 * [1]
    for _ in range(2):
        obj = Tester(
            nval   = 4,
            add_dir= True,
            name   = 'cache_with_dir')
        out = obj.run()

        assert res == out
# -----------------------------------
def test_cache_context(tmp_path : Path):
    '''
    Tests setting caching directory through context manager
    '''
    log.info('')
    res = 4 * [1]

    with Wcache.cache_root(path=tmp_path):
        obj = Tester(nval=4, name='cache_context')
        out = obj.run()

    assert res == out
