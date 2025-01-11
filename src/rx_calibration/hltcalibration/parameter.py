'''
Module holding Parameter class
'''
from collections import UserDict

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_calibration:hltcalibration:parameter')
# ------------------------------------
class Parameter(UserDict):
    '''
    Class meant to hold fitting parameters
    '''
    # ----------------------------------
    def __init__(self):
        super().__init__()
    # ----------------------------------
    def __setitem__(self, name : str, value : tuple[float,float]):
        '''
        Takes the name of the parameter and the tuple with the value and the error
        '''
        log.debug(f'Setting parameter {name} to value {value}')

        super().__setitem__(name, value)
    # ----------------------------------
    def __getitem__(self, name : str) -> tuple[float,float]:
        log.debug(f'Getting value for key {name}')

        return super().__getitem__(name)
# ------------------------------------
