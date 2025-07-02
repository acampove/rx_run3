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
    def __init__(self, nval : int):
        '''
        nval, some integer used to produce output data
        '''
        super().__init__(
                out_path='Tester',
                nval    =nval)

        self._nval    = nval
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

        res = [1] * self._nval
        log.info(f'Data not cached, saving: {res}')

        gut.dump_json(res, obj_path)
        self._cache()

        return res
# -----------------------------------
def test_cache():
    '''
    Tests that value is the correct one when using same inputs
    '''
    log.info('')
    res = 4 * [1]
    for _ in range(2):
        obj = Tester(nval=4)
        out = obj.run()

        assert res == out
# -----------------------------------
def test_update():
    '''
    Tests case where inputs change
    '''
    for val in range(10):
        log.info(f'Testing with: {val}')
        obj = Tester(nval=val)
        out = obj.run()

        assert out == [1] * val
        log.info('')
# -----------------------------------
