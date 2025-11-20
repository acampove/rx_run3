'''
This module contains the ParameterReader class
'''
import os
import math
import pandas as pnd
from pathlib   import Path
from dmu       import Measurement
from dmu       import LogStore
from rx_common import Project, Trigger 
from rx_common import Component
from rx_common import Qsq 
from rx_common import info

log=LogStore.add_logger('fitter:parameter_reader')
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
    def _print_info(self, df : pnd.DataFrame) -> None:
        '''
        This method will print the sample information

        Parameters
        -------------
        df: DataFrame with fit information
        '''
        df_info = df[['brem', 'block', 'q2bin', 'channel']]
        log.info(df_info)
    # ----------------------
    def _query(self, df : pnd.DataFrame, cut : str) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        cut: Selection used to filter dataframe

        Returns
        -------------
        Filtered DataFrame
        '''
        df_out = df.query(cut)

        if len(df_out) == 0:
            self._print_info(df = df)
            raise ValueError(f'No entries pass: {cut}')

        return df_out
    # ----------------------
    def _format_data(self, data : dict[str, float]) -> dict[str,tuple[float,float]]:
        '''
        Parameters
        -------------
        data: Dictionary mapping quantity with value

        Returns
        -------------
        Dictionary mapping quantity with tuple where first element is value and second one is the error
        '''
        output_data : dict[str, list[float]] = dict()
        for key, val in data.items():
            if not isinstance(val, float):
                continue

            if math.isnan(val):
                continue

            if key in ['brem', 'block', 'channel', 'project', 'q2bin', 'mva_cmb', 'mva_prc']:
                continue

            index    = 0 if key.endswith('_value') else 1
            var_name = key.rstrip('_value').rstrip('_error')

            if var_name not in output_data:
                output_data[var_name] = [-999.0, -999.0]

            print(var_name, val)

            output_data[var_name][index] = val

        return { key : (value[0], value[1]) for key, value in output_data.items() } 
    # ----------------------
    def __call__(
        self, 
        brem      : int,
        block     : int,
        component : Component,
        q2bin     : Qsq,
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
