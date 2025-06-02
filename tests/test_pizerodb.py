'''
Module holding tests for PiZeroDb class
'''
import pytest
from dmu.logging.log_store            import LogStore
from rx_calibration.electron.pizerodb import PiZeroDb

log=LogStore.add_logger('rx_calibration:test_pizerodb')
# -----------------------------------------------------
@pytest.mark.parametrize('run_number', [1, 2, 3, 4])
def test_simple(run_number : int):
    '''
    Simplest test
    '''

    db  = PiZeroDb()
    val = db.get_period(run = run_number)

    log.info(val)
# -----------------------------------------------------
