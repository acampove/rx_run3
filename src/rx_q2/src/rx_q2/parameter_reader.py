'''
Module holding the ParameterReader class
'''
import re
import pandas as pnd

from pathlib           import Path
from rx_q2.scales_conf import ScalesConf
from dmu               import LogStore
from dmu.generic       import utilities as gut

log=LogStore.add_logger('rx_q2:parameter_reader')
# ----------------------
class ParameterReader:
    '''
    Class meant to extract information from JSON files
    created from fits to the B and Jpsi mass distributions
    '''
    # ----------------------
    def __init__(self, cfg : ScalesConf):
        '''
        Parameters
        -------------
        cfg: Object holding configuration
        '''
        self._cfg = cfg

    # ----------------------
    def read(self, path : Path) -> pnd.Series:
        '''
        Parameters
        -------------
        path: Path to JSON file with fitting parameters

        Returns
        -------------
        Pandas series with fit parameters information
        '''
        return pnd.Series()
# ----------------------
