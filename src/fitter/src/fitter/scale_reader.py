'''
Module holding ScaleReader class
'''
import os
import pandas as pnd
from dmu       import LogStore
from pathlib   import Path
from rx_common import Block, Brem, Correction

log=LogStore.add_logger('fitter:scale_reader')
# -------------------------------------
class ScaleReader:
    '''
    Class in charge of accessing mass scales, resolutions, brem fractions, etc
    and making them available
    '''
    # ----------------------
    def __init__(self) -> None:
        '''

        '''
        ana_dir   = Path(os.environ['ANADIR'])
        data_path = ana_dir / 'fits/data/reso_non_dtf/v1/rk_ee/plots/scales.json'
        if not data_path.exists():
            raise FileNotFoundError(f'Cannot find scales file: {data_path}')

        self._df  = pnd.read_json(data_path)
    # ----------------------
    def get_scale(
        self,
        corr  : Correction,
        block : Block,
        brem  : Brem) -> tuple[float,float]:
        '''
        Parameters
        -------------
        corr : Type of correction
        block: E.g. 1...8
        brem : E.g. 1, 2

        Returns
        -------------
        Tuple with value and error of scale
        '''
        df = self._df
        df = df.query(f'block == {block}')
        df = df.query(f'brem  == {brem}')

        if len(df) != 1:
            log.error(df)
            raise ValueError(f'Not found one and only one row for block/brem: {block}/{brem}')

        sr  = df.iloc[0]
        val = sr[f'{corr}_val']
        err = sr[f'{corr}_err']

        return val, err 
# -------------------------------------
