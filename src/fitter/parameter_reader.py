'''
This module contains the ParameterReader class
'''
import os
import pandas as pnd
from dmu       import Measurement
from pathlib   import Path
from rx_common import Trigger 
from rx_common import Component

# ------------------------------------
class FitMeasurement(Measurement):
    '''
    Class meant to represent the measurements associated to a fit
    i.e. the fit parameters
    '''
    @property
    def candidates(self) -> tuple[float,float]:
        return 1, 1
# ------------------------------------
class ParameterReader:
    '''
    This class is meant to be an interface to the dataframe
    containing the fitting parameters
    '''
    # ----------------------
    def __init__(self, name : str):
        '''
        Parameters
        -------------
        name : Fit name, e.g. mid_window 
        '''
        ana_dir   = Path(os.environ['ANADIR'])
        pars_path = ana_dir / f'fits/data/{name}/parameters.parquet'
        if not pars_path.exists():
            raise FileNotFoundError(f'Cannot find: {pars_path}')

        self._df = pnd.read_parquet(path = pars_path)
    # ----------------------
    def __call__(
        self, 
        brem      : int,
        block     : int,
        component : Component,
        trigger   : Trigger) -> FitMeasurement:
        '''
        Parameters
        -------------
        brem     : Brem category, e.g. 0, 1, 2
        block    : Block number in 2024, e.g. 1, 2, 3...8
        component: Name of fitting component
        trigger  : HLT2 trigger, used to pick channel, etc

        Returns
        -------------
        FitMeasurement instance, i.e. container with parameter names, values and errors
        '''
        data = {'a' : (1., 1.)}

        return FitMeasurement(data = data)
        
# ------------------------------------
