'''
Module holding ScaleCombiner class
'''
import os
import pandas as pnd
from pathlib import Path

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_q2:scale_combiner')
# -------------------------------------
class ScaleCombiner:
    '''
    Class in charge of:

    - Load scales from each sample, given a version
    - Combine them statisticall
    - Write combined scales to JSON file
    '''
    # ----------------------
    def __init__(self, version : str) -> None:
        '''
        Parameters
        -------------
        version: Version of fits, e.g. v2
        '''
        self._version = version
    # ----------------------
    def _get_dataframes(self, measurement : str) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        measurement: Directory where measurement is found, e.g. rk_ee

        Returns
        -------------
        DataFrame with the scales
        '''
        ana_dir = Path(os.environ['ANADIR'])
        jsn_path= ana_dir / f'q2/fits/{self._version}/plots/{measurement}/parameters.json'

        if not jsn_path.exists():
            raise FileNotFoundError(f'Cannot find {jsn_path}')

        log.info(f'Getting dataframe for: {measurement}')

        df = pnd.read_json(jsn_path)
        df = df.sort_values(by=['brem', 'block', 'sample'])
        df = df.reset_index(drop=True)

        log.warning('Dropping blocks 7 and 8')
        df = df[~df['block'].isin([7, 8])]

        log.debug(df)

        return df
    # ----------------------
    def _combine(self, l_df : list[pnd.DataFrame]) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        l_df: List of dataframes with scales to be combined

        Returns
        -------------
        Dataframe with combination
        '''
        log.info('Combining measurements')
    def _validate(self, l_df : list[pnd.DataFrame]) -> None:
        '''
        Validates dataframes

        Parameters
        -------------
        l_df: List of dataframes with scales to combine
        '''
        nmeas = len(l_df)
        if nmeas != 2:
            raise ValueError(f'Can only combine two measurements, found {nmeas}')

        log.info(f'Combining {nmeas} measurements')

        nrows = { len(df)         for df in l_df }
        ncols = { len(df.columns) for df in l_df }

        if len(nrows) != 1:
            raise ValueError(f'Multiple sizes for dataframes: {nrows}')

        if len(ncols) != 1:
            raise ValueError(f'Multiple sizes for dataframes: {ncols}')

        log.debug(f'All dataframes have same number of rows/columns: {nrows}/{ncols}')
    # ----------------------
    def _save_combination(
        self, 
        name: str,
        df  : pnd.DataFrame) -> None:
        '''
        Saves combination to JSON file

        Parameters
        -------------
        name: Name of JSON file where combination is saved
        df  : DataFrame with combined scales
        '''
        ana_dir = Path(os.environ['ANADIR'])
        jsn_path= ana_dir / f'q2/fits/{self._version}/{name}'

        log.info(f'Saving combination to: {jsn_path}')

        df.to_json(jsn_path, indent=2)
    # ----------------------
    def combine(
        self, 
        name         : str,
        measurements : list[str]) -> None:
        '''
        Writes the combined scales to JSON

        Parameters
        -------------
        name: Name of file where combination is stored
        measurements: List of strings with names of directories with measurements to combine, e.g. rk_ee
        '''
        l_df = [ self._get_dataframes(measurement=measurement) for measurement in measurements ]
        self._validate(l_df = l_df)

        df   = self._combine(df_1=l_df[0], df_2=l_df[1])

        self._save_combination(name=name, df=df)
# -------------------------------------
