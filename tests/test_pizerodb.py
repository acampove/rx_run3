'''
Module holding tests for PiZeroDb class
'''
import pytest
from dmu.logging.log_store            import LogStore
from conftest                         import ConfData
from rx_calibration.electron.pizerodb import PiZeroDb

log=LogStore.add_logger('rx_calibration:test_pizerodb')
# -----------------------------------------------------
class Data:
    '''
    Data class
    '''
    db  = PiZeroDb()
# -----------------------------------------------------
@pytest.mark.parametrize('run', ConfData.l_run)
def test_simple(run : int):
    '''
    Simplest test
    '''
    log.info('Simple')

    val = Data.db.get_period(run = run)

    log.info(val)
# -----------------------------------------------------
