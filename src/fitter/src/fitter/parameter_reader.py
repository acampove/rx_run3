'''
This module contains the ParameterReader class
'''
import os
import math
import pandas as pnd
from contextlib import contextmanager
from pathlib    import Path
from dmu        import LogStore
from dmu.stats  import Measurement
from rx_common  import Project, Trigger 
from rx_common  import Qsq 
from rx_common  import info

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
    _pars_path : Path | None = None
    # ----------------------
    def __init__(self, name : str | None = None):
        '''
        Parameters
        -------------
        name : Fit name, e.g. mid_window 
        '''
        if name is None and self._pars_path is None:
            raise ValueError('No name was passed and no path to parameters exists')

        self._df = self._get_dataframe(name = name)
    # ----------------------
    def _get_dataframe(self, name : str | None = None) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        name: Label for fits

        Returns
        -------------
        Dataframe with fit parameters 
        '''
        if self._pars_path:
            log.info(f'Using user defined path: {self._pars_path}')
            return pnd.read_parquet(self._pars_path)

        ana_dir   = Path(os.environ['ANADIR'])
        pars_path = ana_dir / f'fits/data/{name}/parameters.parquet'

        log.debug(f'Using path: {pars_path}')
        if not pars_path.exists():
            raise FileNotFoundError(f'Cannot find: {pars_path}')

        return pnd.read_parquet(path = pars_path)
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
    def _format_data(
        self, 
        brem : int,
        data : dict[str, float]) -> dict[str,tuple[float,float]]:
        '''
        Parameters
        -------------
        brem: Brem category
        data: Dictionary mapping quantity with value, value is a float, i.e. dictionary has been filtered

        Returns
        -------------
        Dictionary mapping quantity with tuple where first element is value and second one is the error
        '''
        output_data : dict[str, list[float]] = dict()
        for key, val in data.items():
            if math.isnan(val):
                continue

            if key in ['brem', 'block', 'channel', 'project', 'q2bin', 'mva_cmb', 'mva_prc']:
                continue

            index    = 0 if key.endswith('_value') else 1
            var_name = key.rstrip('_value').rstrip('_error')

            if var_name not in output_data:
                output_data[var_name] = [-999.0, -999.0]

            output_data[var_name][index] = val

        res = { key : (value[0], value[1]) for key, value in output_data.items() } 

        name, val, err = self._extract_brem(brem = brem, data = data)

        res[name] = val, err

        return res
    # ----------------------
    def _extract_brem(self, brem : int, data : dict[str, float]) -> tuple[str,float,float]:
        '''
        Parameters
        -------------
        brem: brem category, i.e. 1 or 2
        data: Dictionary with variable name and value

        Returns
        -------------
        Tuple with value of brem fraction, error and name of variable
        '''
        keys = [ key for key in data if 'fraction' in key ]
        if len(keys) != 2:
            for key in data:
                log.error(key)
            raise ValueError('Expected two entries corresponding to brem fraction')

        [val] = [ data[key] for key in keys if key.endswith('_value') ]
        [err] = [ data[key] for key in keys if key.endswith('_error') ]

        if brem == 2:
            val = 1 - val # fraction stored is brem 1 one

        return f'fr_brem_{brem:03d}', val, err
    # ----------------------
    def __call__(
        self, 
        brem      : int,
        block     : int,
        cmb       : str,
        prc       : str,
        kind      : str,
        project   : Project,
        q2bin     : Qsq,
        trigger   : Trigger) -> FitMeasurement:
        '''
        Parameters
        -------------
        cmb/prc  : String specifying working point, e.g. 050
        kind     : Either dat or sim
        brem     : Brem category, e.g. 0, 1, 2
        block    : Block number in 2024, e.g. 1, 2, 3...8
        component: Name of fitting component
        trigger  : HLT2 trigger, used to pick channel, etc

        Returns
        -------------
        FitMeasurement instance, i.e. container with parameter names, values and errors
        '''
        channel = info.channel_from_trigger(trigger=trigger, lower_case = True)
        df      = self._df

        df      = self._query(df = df, cut = f'brem    ==   {brem}'     )
        df      = self._query(df = df, cut = f'block   ==   {block}'    )
        df      = self._query(df = df, cut = f'kind    == \"{kind}\"'   )
        df      = self._query(df = df, cut = f'mva_cmb == \"{cmb}\"'    )
        df      = self._query(df = df, cut = f'mva_prc == \"{prc}\"'    )
        df      = self._query(df = df, cut = f'q2bin   == \"{q2bin}\"'  )
        df      = self._query(df = df, cut = f'channel == \"{channel}\"')
        df      = self._query(df = df, cut = f'project == \"{project}\"')

        if len(df) != 1:
            self._print_info(df=df)
            raise ValueError('Not found one and only one row')

        raw_data = df.iloc[0].to_dict()
        flt_data = { key : value for key, value in raw_data.items() if isinstance(value, float) }
        data     = self._format_data(
            brem = brem,
            data = flt_data) # type: ignore

        # for simultaneous fits, this removes brem_002 parameters when one needs brem_001
        # and viceversa
        data     = { key : value for key, value in data.items() if f'brem_{brem:03d}' in key }

        return FitMeasurement(data = data)
    # ----------------------
    @classmethod
    def inputs_from(cls, pars_path : Path):
        '''
        Parameters
        -------------
        pars_path: Path to parquet file with parameters 

        Returns
        -------------
        Context manager
        '''
        old_val = cls._pars_path

        @contextmanager
        def _context():
            try:
                cls._pars_path = pars_path
                yield
            finally:
                cls._pars_path = old_val

        return _context()
# ------------------------------------
