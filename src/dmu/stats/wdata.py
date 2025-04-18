'''
Module with Wdata class
'''
import zfit
import numpy
from zfit.core.interfaces  import ZfitSpace  as zobs
from zfit.core.data        import Data       as zdata

from dmu.logging.log_store import LogStore

# pylint: disable=too-many-ancestors

log=LogStore.add_logger('dmu:stats:wdata')
# -------------------------------
class Wdata:
    '''
    Class meant to symbolize weighted data
    '''
    # -------------------------------
    def __init__(self, data : numpy.ndarray, weights : numpy.ndarray):
        self._data    = data
        self._weights = weights
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
