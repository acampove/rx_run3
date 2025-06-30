'''
Module used to test Cache class
'''
import pytest

from dmu.generic           import utilities as gut
from dmu.workflow.cache    import Cache     as Wcache
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:workflow:test_cache')
# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:workflow:cache', 10)
# -----------------------------------
class Tester(Wcache):
    '''
    Testing class, produces outputs from simple inputs
    '''
    # -----------------------------------
    def __init__(
            self,
            nval    : int,
            out_dir : str):
        '''
        nval, some integer used to produce output data
        '''
        super().__init__(
                out_path=out_dir,
                nval    =nval,
                out_dir =out_dir)

        self._out_dir = out_dir
        self._nval    = nval
    # -----------------------------------
    def run(self) -> list[int]:
        '''
        Returns a list of 1's
        '''
        obj_path = f'{self._out_dir}/values.json'

        if self._copy_from_cache():
            log.warning('Output cached, not running')
            return gut.load_json(obj_path)

        log.info('Data not cached, running')
        res = [1] * self._nval

        gut.dump_json(res, obj_path)
        self._cache()

        return res
# -----------------------------------
def test_cache():
    '''
    Simplest test
    '''
    res = None
    odir= '/tmp/tests/dmu/workflow/cache/cache'

    log.info('')
    for _ in range(5):
        obj = Tester(nval=10, out_dir=odir)
        obj.run()

        out = gut.load_json(path=f'{odir}/values.json')

        if res is None:
            res = out
            continue

        assert res == out

        res = out
# -----------------------------------
def test_update():
    '''
    Tests
    '''
    odir = '/tmp/tests/dmu/workflow/cache/update'

    for val in range(10):
        obj = Tester(nval=val, out_dir=odir)
        obj.run()

        out = gut.load_json(path=f'{odir}/values.json')

        assert out == [1] * val
        log.info('')
# -----------------------------------
