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
    def get_scale(
        self,
        name : str,
        block : str,
        brem  : str) -> tuple[float,float]:
        '''
        Parameters
        -------------
        name: Kind of scale, e.g. scale, reso, brem
        block: E.g. 1...8
        brem : E.g. 1, 2

        Returns
        -------------
        Tuple with value and error of scale
        '''
        return 1, 0
# -------------------------------------
