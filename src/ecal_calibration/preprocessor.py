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

        self._brem_cut = 'L1_brem + L2_brem == 1'
    # ---------------------------------
    def _apply_selection(self, ddf : DDF) -> DDF:
        ddf = ddf.query(self._brem_cut)
        if 'selection' not in self._cfg:
            return ddf

        for selection in self._cfg['selection']:
            ddf = ddf.query(selection)

        return ddf
    # ---------------------------------
    def _get_correction(self, row : pnd.Series, lepton : str) -> float:
        return 1.0
    # ---------------------------------
    def _build_features(self, row_sr : pnd.Series) -> pnd.Series:
        lep = 'L1' if row_sr['L1_brem'] == 1 else 'L2'

        data        = {}
        data['row'] = row_sr[f'{lep}_BREMHYPOROW']
        data['col'] = row_sr[f'{lep}_BREMHYPOCOL']
        data['are'] = row_sr[f'{lep}_BREMHYPOAREA']
        data['eng'] = row_sr[f'{lep}_BREMTRACKBASEDENERGY']
        data['npv'] = row_sr['nPVs']
        data['blk'] = row_sr['block']
        data['mu' ] = self._get_correction(row=row_sr, lepton=lep)

        row_sr = pnd.Series(data)

        return row_sr
    # ---------------------------------
    def get_data(self) -> DDF:
        '''
        Returns dask dataframe after preprocessing
        '''
        ddf = self._apply_selection(ddf=self._ddf)

        ddf_feat = ddf.apply(self._build_features, axis=1)

        return ddf_feat
# --------------------------
