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
        super().__init__()

        self._register(nval=nval, out_dir=out_dir)
        self._set_output(dir_path=out_dir)

        self._out_dir = out_dir
        self._nval    = nval
    # -----------------------------------
    def run(self) -> None:
        '''
        Returns a list of 1's
        '''
        if self._is_cached():
            log.warning('Output cached, not running')
            return

        log.info('Data not cached, running')
        res = [1] * self._nval

        gut.dump_json(res, f'{self._out_dir}/values.json')

        self._mark_as_cached()
# -----------------------------------
def test_cache():
    '''
    Simplest test
    '''
    res = None
    odir= '/tmp/tests/dmu/workflow/cache/cache'

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
