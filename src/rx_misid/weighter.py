'''
Module with SampleWeighter class
'''
import numpy
import pandas as pnd
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
    def __init__(self, df : pnd.DataFrame, cfg : dict):
        '''
        df  : Pandas dataframe with columns 'hadron', 'bmeson' and 'kind'. Used to assign weights
        cfg : Dictionary storing configuration
        '''
        self._df  = df
        self._cfg = cfg
    # ------------------------------
    def _get_weights(self, df : pnd.DataFrame) -> numpy.ndarray:
        log.warning('Using ones as weights')
        return numpy.ones(len(df))
    # ------------------------------
    def _weight_data(self, kind : str, df : pnd.DataFrame) -> pnd.DataFrame:
        log.info(f'Applying weights to {kind}')

        arr_wgt       = self._get_weights(df=df)
        df['weights'] = arr_wgt

        return df
    # ------------------------------
    def get_weighted_data(self) -> pnd.DataFrame:
        '''
        Returns instance of weighted data
        '''
        l_df_wgt = []
        for kind, df_kind in self._df.groupby('kind'):
            df_wgt = self._weight_data(kind, df_kind)
            l_df_wgt.append(df_wgt)

        df = pnd.concat(l_df_wgt)

        return df
# ------------------------------
