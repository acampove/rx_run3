'''
Module holding Parameter class
'''
import os
import json
from collections import UserDict

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_calibration:hltcalibration:parameter')
# ------------------------------------
class Parameter(UserDict):
    '''
    Class meant to hold fitting parameters
    '''
    # ----------------------------------
    def __setitem__(self, name : str, value : tuple[float,float]):
        '''
        Takes the name of the parameter and the tuple with the value and the error
        '''
        log.debug(f'Setting parameter {name} to value {value}')

        super().__setitem__(name, value)
    # ----------------------------------
    def __getitem__(self, name : str) -> tuple[float,float]:
        '''
        Takes name of parameter, returns value and error tuple
        '''
        log.debug(f'Getting value for key {name}')

        if name not in self.data:
            raise ValueError(f'Variable {name} not found')

        return super().__getitem__(name)
    # ----------------------------------
    def save(self, path : str) -> None:
        '''
        Will save current object to JSON, using the path as argument
        '''
        dir_path = os.path.dirname(path)
        os.makedirs(dir_path, exist_ok=True)

        with open(path, 'w', encoding='utf-8') as ofile:
            json.dump(self.data, ofile, indent=4)
# ------------------------------------
