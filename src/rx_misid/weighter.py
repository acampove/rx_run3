'''
Module with SampleWeighter class
'''
import numpy
from dmu.stats.wdata           import Wdata
from dmu.logging.log_store     import LogStore

log=LogStore.add_logger('rx_misid:weighter')
# ------------------------------
class SampleWeighter:
    '''
    Class intended to:

    - Read calibration maps
    - Pick datasets
    - Apply weights to datasets and return them
    '''
    # ------------------------------
    def __init__(self, samples : dict[str,Wdata], cfg : dict):
        '''
        samples : Dictionary mapping kind of map (PassFail, etc) with datasets
        cfg     : Dictionary storing configuration
        '''
        self._samples = samples
        self._cfg     = cfg
    # ------------------------------
    def _get_weights(self, data : Wdata) -> numpy.ndarray:
        log.warning('Using ones as weights')
        return numpy.ones(data.size)
    # ------------------------------
    def _weight_data(self, kind : str, data : Wdata) -> Wdata:
        log.info(f'Applying weights to {kind}')

        arr_wgt = self._get_weights(data=data)
        data    = data.update_weights(weights=arr_wgt, replace=False)

        return data
    # ------------------------------
    def get_weighted_data(self) -> dict[str,Wdata]:
        '''
        Returns instance of weighted data
        '''

        d_data_weighted = { kind : self._weight_data(kind, data) for kind, data in self._samples.items() }

        return d_data_weighted
# ------------------------------
