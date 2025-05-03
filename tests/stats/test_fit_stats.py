'''
File with testing functions for FitStats class
'''
import os
import pytest

from dmu.logging.log_store  import LogStore
from dmu.stats              import utilities
from dmu.stats.fit_stats    import FitStats

log = LogStore.add_logger('dmu:test_fit_stats')
# -------------------------------------------------
class Data:
    '''
    data class
    '''
    fit_dir = '/tmp/tests/dmu/stats/fit_stats'
# -------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.fit_dir, exist_ok=True)
    utilities.placeholder_fit(kind='s+b', fit_dir=Data.fit_dir)

    LogStore.set_level('dmu:test_fit_stats', 10)
    LogStore.set_level('dmu:fit_stats'     , 10)
# -------------------------------------------------
@pytest.mark.parametrize('var' , ['nsig'])
@pytest.mark.parametrize('kind', ['value', 'error'])
def test_simple(var : str, kind : str):
    '''
    Retrieve signal yield
    '''
    obj =FitStats(fit_dir=Data.fit_dir)
    val = obj.get_value(name=var, kind=kind)

    log.info(f'{kind:<20}{var:<20}{val:.3f}')
# -------------------------------------------------
def test_print_blind():
    '''
    Retrieve signal yield
    '''
    obj =FitStats(fit_dir=Data.fit_dir)
    obj.print_blind_stats()
# -------------------------------------------------
