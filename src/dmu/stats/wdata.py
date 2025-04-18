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
    def to_zfit(self, obs : zobs) -> zdata:
        '''
        Function that takes a zfit observable and uses it
        to build a zfit data instance tha it then returns
        '''
        log.debug('Building zfit dataset')

        data = zfit.data.Data(obs=obs, data=self._data, weights=self._weights)

        return data
# -------------------------------
