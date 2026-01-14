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
    #-------------------------------------
    def _row_from_path(self, path : Path) -> pnd.Series:
        '''
        Parameters
        -----------------
        path: Full path to parameters.json
    
        Returns
        -----------------
        Pandas series with extracted information meant to be used as a row
        '''
        data = gut.load_json(path)
    
        [[mu_val, mu_err]] = [ val for name, val in data.items() if name.startswith('mu_')]
        [[sg_val, sg_err]] = [ val for name, val in data.items() if name.startswith('sg_')]
    
        brem, block = self._brem_block_from_path(path=path)

        data = {
            'mu_val' : mu_val,
            'mu_err' : mu_err,
            'sg_val' : sg_val,
            'sg_err' : sg_err,
            'brem'   : brem,
            'block'  : block,
        } 
    
        return pnd.Series(data) 
    #-------------------------------------
    def _brem_block_from_path(self, path : Path) -> tuple[str,str]:
        '''
        Parameters
        ---------------
        Path: path to parameters.json
    
        Returns
        ---------------
        Tuple with strings describing the brem and block
        '''
        mtch = re.match(self._cfg.regex, path.parent.name)
        if not mtch:
            raise ValueError(f'Cannot extract information from {path.parent.name} using {self._cfg.regex}')
    
        [brem, block] = mtch.groups()
    
        return brem, block
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
        return self._row_from_path(path = path)
# ----------------------
