'''
Module holding tests for PiZeroDb class
'''
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
def test_simple():
    '''
    Simplest test
    '''
    l_run = ConfData.l_run

    log.info(f'Running simple test with {len(l_run)} runs')
    for run in l_run:
        period = Data.db.get_period(run = run)
        ConfData.add_run_period(run, period)
# -----------------------------------------------------
