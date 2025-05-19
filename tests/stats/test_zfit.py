'''
Module testing zfit wrapper
'''
import os

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:test_zfit')
# --------------------------------------------
def test_import():
    '''
    Tests basic import
    '''

    from dmu.stats.zfit import zfit

    zfit_path = zfit.__file__
    assert os.path.isfile(zfit_path)

    log.info(f'zfit found in: {zfit_path}')
# --------------------------------------------
