'''
Module with Wdata class
'''
from typing import Union

import zfit
import numpy
import pandas as pnd
from zfit.core.interfaces  import ZfitSpace  as zobs
from zfit.core.data        import Data       as zdata

from dmu.logging.log_store import LogStore


log=LogStore.add_logger('dmu:stats:wdata')
# -------------------------------
class Wdata:
    '''
    Class meant to symbolize weighted data
    '''
    # -------------------------------
    def __init__(self,
                 data          : Union[numpy.ndarray, pnd.DataFrame],
                 weights       : numpy.ndarray = None,
                 extra_columns : pnd.DataFrame = None):
        '''
        data   :
        weights: Numpy array with weights, if not passed, will use ones as weights
        extra_columns: Extra information that can be attached to the Wdata object in the form of a pandas dataframe, default None
        '''
        self._data    = data
        self._weights = self._get_weights(weights)
        self._df_extr = self._get_df_extr(extra_columns)
    # -------------------------------
    def _get_df_extr(self, df : pnd.DataFrame) -> pnd.DataFrame:
        if not isinstance(df, pnd.DataFrame):
            arg_type = type(df)
            raise ValueError(f'Expected a pandas dataframe, got {arg_type}')

        if len(df) != self.size:
            raise ValueError('Input dataframe differs in length from data')

        return df
    # -------------------------------
    def _get_weights(self, weights : numpy.ndarray) -> numpy.ndarray:
        if weights is None:
            log.info('Weights not found, using ones')
            return numpy.ones(self.size)

        if not isinstance(weights, numpy.ndarray):
            raise ValueError('Weights argument is not a numpy array')

        weights_size = len(weights)
        if weights_size != self.size:
            raise ValueError(f'Data size and weights size differ: {self.size} != {weights_size}')

        return weights
    # -------------------------------
    def _build_new_array(self, arr_other : numpy.ndarray, kind : str) -> numpy.ndarray:
        arr_this = getattr(self, kind)
        arr      = numpy.concatenate([arr_this, arr_other])

        return arr
    # -------------------------------
    def __add__(self, other : 'Wdata') -> 'Wdata':
        '''
        Takes instance of Wdata and adds it to this instance
        returning sum.

        Addition is defined as concatenating both data and weights.
        '''
        if not isinstance(other, Wdata):
            other_type = type(other)
            raise NotImplementedError(f'Cannot add Wdata instance to {other_type} instance')

        log.debug('Adding instances of Wdata')
        data    = self._build_new_array(arr_other=other._data   , kind='_data'   )
        weights = self._build_new_array(arr_other=other._weights, kind='_weights')

        return Wdata(data=data, weights=weights)
    # -------------------------------
    def __eq__(self, other) -> bool:
        '''
        Checks that the data and weights are the same within a 1e-5 relative tolerance
        '''
        rtol = 1e-5
        equal_data    = numpy.allclose(other._data   , self._data   , rtol=rtol)
        equal_weights = numpy.allclose(other._weights, self._weights, rtol=rtol)

        return equal_data and equal_weights
    # -------------------------------
    @property
    def size(self) -> int:
        '''
        Returns number of entries in dataset
        '''
        return len(self._data)
    # -------------------------------
    @property
    def sumw(self) -> int:
        '''
        Returns sum of weights
        '''
        return numpy.sum(self._weights)
    # -------------------------------
    def update_weights(self, weights : numpy.ndarray, replace : bool) -> 'Wdata':
        '''
        Takes array of weights to either:
        - Replace existing array
        - Update by multiply by existing array

        depending on the replace argument value. It returns a new instance of Wdata
        '''
        if self._weights.shape != weights.shape:
            raise ValueError(f'Invalid shape for array of weights, expected/got: {self._weights.shape}/{weights.shape}')

        new_weights = weights if replace else weights * self._weights

        data = Wdata(data=self._data, weights=new_weights)

        return data
    # -------------------------------
    def to_zfit(self, obs : zobs) -> zdata:
        '''
        Function that takes a zfit observable and uses it
        to build a zfit data instance tha it then returns
        '''
        log.debug('Building zfit dataset')

        data = zfit.data.Data(obs=obs, data=self._data, weights=self._weights)

        return data
# -------------------------------
