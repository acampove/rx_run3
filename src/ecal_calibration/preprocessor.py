'''
Module holding PreProcessor class
'''
from contextlib import contextmanager

import torch
import pandas as pnd

from torch          import tensor
from vector         import MomentumObject3D as v3d
from dask.dataframe import DataFrame        as DDF

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ecal_calibration:preprocessor')
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

    The targer will be called "mu" and will represent the correction needed
    '''
    # ---------------------------------
    def __init__(self, ddf : DDF, cfg : dict):
        '''
        ddf: Dask dataframe with raw data to preprocess
        '''
        self._ddf      = ddf
        self._cfg      = cfg
        self._brem_cut = '(L1_brem + L2_brem > 0.5) & (L1_brem + L2_brem < 1.5)'
        self._neg_tol  = -10
        self._ddf_res  : DDF
        self._nentries = None
    # ---------------------------------
    def _apply_selection(self, ddf : DDF) -> DDF:
        log.info(f'Applying: {self._brem_cut}')
        ddf = ddf.query(self._brem_cut)

        if 'selection' not in self._cfg:
            return ddf

        for selection in self._cfg['selection']:
            log.info(f'Applying: {selection}')
            ddf = ddf.query(selection)

        return ddf
    # ---------------------------------
    def _values(self, kind : str) -> tensor:
        ddf = self.get_data()
        if self._nentries is None:
            df  = ddf.compute()
        else:
            df  = ddf.head(self._nentries)

        if   kind == 'features':
            l_col = [ var for var in df.columns if var != 'mu' ]
        elif kind == 'targets':
            l_col = [ var for var in df.columns if var == 'mu' ]
        else:
            raise ValueError(f'Invalid kind of value: {kind}')

        arr_val = df[l_col].to_numpy()

        return torch.tensor(arr_val, dtype=torch.float32)
    # ---------------------------------
    def get_data(self, partition_size : str = '10MB') -> DDF:
        '''
        Returns dask dataframe after preprocessing, it contains.

        - The features in the class description.
        - The target for regression, labeled as 'mu'

        Parameters
        ----------------------
        partition_size : Partition size in MB. Used to split among workers.
        '''
        if hasattr(self, '_ddf_res'):
            return self._ddf_res

        ddf = self._ddf.dropna()
        ddf = ddf.repartition(partition_size=partition_size)
        ddf = self._apply_selection(ddf=ddf)

        # This meta thing MUST be here
        # Or else dask will try to "guess"
        # the structure by plugging in data
        # that will make the code rise exception
        meta = {
            'row': 'f8',
            'col': 'f8',
            'are': 'f8',
            'eng': 'f8',
            'npv': 'i8',
            'blk': 'i8',
            'mu' : 'f8'
        }

        ddf = ddf.apply(
                PreProcessor.build_features,
                meta = meta,
                axis = 1)

        self._ddf_res = ddf

        return ddf
    # ---------------------------------
    @property
    def features(self) -> tensor:
        '''
        Returns pytorch tensor with features
        '''

        return self._values(kind='features')
    # ---------------------------------
    @property
    def targets(self) -> tensor:
        '''
        Returns pytorch tensor with values of regression targets
        '''

        return self._values(kind='targets')
    # ---------------------------------
    @contextmanager
    def max_entries(self, nentries : int):
        '''
        Context manager that will
        set, temporarily, the number of entries, e.g for tests
        '''
        old            = self._nentries
        self._nentries = nentries
        try:
            yield
        finally:
            self._nentries = old
    # ---------------------------------
    @staticmethod
    def _get_normal(row : pnd.Series) -> v3d:
        pvx = row['B_BPVX']
        pvy = row['B_BPVY']
        pvz = row['B_BPVZ']

        svx = row['B_END_VX']
        svy = row['B_END_VY']
        svz = row['B_END_VZ']

        dr  = v3d(x=svx - pvx, y=svy - pvy, z=svz - pvz)

        return dr / dr.mag
    # ---------------------------------
    @staticmethod
    def _get_momentum(row : pnd.Series, name : str) -> v3d:
        pt = row[f'{name}_PT' ]
        et = row[f'{name}_ETA']
        ph = row[f'{name}_PHI']

        return v3d(pt=pt, eta=et, phi=ph)
    # ---------------------------------
    @staticmethod
    def _get_correction(row : pnd.Series, lepton : str) -> float:
        norm = PreProcessor._get_normal(row=row)
        l1_p = PreProcessor._get_momentum(row=row, name='L1')
        l2_p = PreProcessor._get_momentum(row=row, name='L2')
        kp_p = PreProcessor._get_momentum(row=row, name= 'H')

        # Remove the component alongside normal. i.e. vectors lie on plane
        l1_p = l1_p - norm * norm.dot(l1_p)
        l2_p = l2_p - norm * norm.dot(l2_p)
        kp_p = kp_p - norm * norm.dot(kp_p)

        if   lepton == 'L1':
            lep = l1_p
            oth = l2_p + kp_p
        elif lepton == 'L2':
            lep = l2_p
            oth = l1_p + kp_p
        else:
            raise ValueError(f'Invalid lepton: {lepton}')

        a = lep.dot(oth)
        b = lep.mag ** 2

        return - a/b
    # ---------------------------------
    @staticmethod
    def build_features(
            row         : pnd.Series,
            lep         : str  = None,
            skip_target : bool = False) -> pnd.Series:
        '''
        Builds features and optionally target, needed for training.

        Parameters:
        row        : Pandas series with information from tree branches, one entry.
        lep        : By default None, if passed as L1/L2, it will target this lepton to buld features
        skip_target: By default false, if True it will not calculate "mu". Recommended for prediction mode

        Returns series with features and target needed for training or prediction
        '''
        log.debug('Building features')

        if   lep is not None:
            log.debug(f'Picking up user defined lepton {lep}')
        elif row['L1_brem'] == 1 and row['L2_brem'] != 1:
            lep = 'L1'
        elif row['L1_brem'] != 1 and row['L2_brem'] == 1:
            lep = 'L2'
        else:
            log.info(row)
            raise ValueError('One and only one electron must have brem')

        data        = {}
        data['nam'] = lep
        data['row'] = row[f'{lep}_BREMHYPOROW']
        data['col'] = row[f'{lep}_BREMHYPOCOL']
        data['are'] = row[f'{lep}_BREMHYPOAREA']
        data['eng'] = row[f'{lep}_BREMTRACKBASEDENERGY'] / 1000 # Very large numbers seem to break down training
        data['npv'] = row['nPVs']
        data['blk'] = row['block']
        data['evn'] = row['EVENTNUMBER']

        if not skip_target:
            data['mu' ] = 1000 * PreProcessor._get_correction(row=row, lepton=lep)

        row = pnd.Series(data)

        return row
# --------------------------
