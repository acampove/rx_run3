'''
Module holding PreProcessor class
'''

import pandas as pnd
from dask.dataframe import DataFrame as DDF

# --------------------------
class PreProcessor:
    '''
    Class used to process input data into features and target columns.
    The features needed are:

    - row : Row of brem photon in ECAL
    - col : Calumn of brem photon in ECAL
    - area: Index of region of ECAL, 0, 1, 2
    - energy: Energy of brem photon
    - npvs  : Number of primary vertices
    - block : Index representing part of the year when data was collected

    The label will be called "mu"
    '''
    # ---------------------------------
    def __init__(self, ddf : DDF, cfg : dict):
        '''
        ddf: Dask dataframe with raw data to preprocess
        '''
        self._ddf = ddf
        self._cfg = cfg
    # ---------------------------------
    def _apply_selection(self, ddf : DDF) -> DDF:
        for selection in self._cfg['selection']:
            ddf = ddf.query(selection)

        return ddf
    # ---------------------------------
    def get_data(self) -> DDF:
        '''
        Returns dask dataframe after preprocessing
        '''
        ddf = self._apply_selection(ddf=self._ddf)

        return ddf
# --------------------------
