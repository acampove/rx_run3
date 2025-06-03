'''
Module holding PreProcessor class
'''
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
        self._brem_cut = 'L1_brem + L2_brem == 1'
        self._neg_tol  = -10
        self._ddf_res  : DDF
    # ---------------------------------
    def _apply_selection(self, ddf : DDF) -> DDF:
        ddf = ddf.query(self._brem_cut)
        if 'selection' not in self._cfg:
            return ddf

        for selection in self._cfg['selection']:
            ddf = ddf.query(selection)

        return ddf
    # ---------------------------------
    def _get_normal(self, row : pnd.Series) -> v3d:
        pvx = row['B_BPVX']
        pvy = row['B_BPVY']
        pvz = row['B_BPVZ']

        svx = row['B_END_VX']
        svy = row['B_END_VY']
        svz = row['B_END_VZ']

        dr  = v3d(x=svx - pvx, y=svy - pvy, z=svz - pvz)

        return dr / dr.mag
    # ---------------------------------
    def _get_momentum(self, row : pnd.Series, name : str) -> v3d:
        pt = row[f'{name}_PT' ]
        et = row[f'{name}_ETA']
        ph = row[f'{name}_PHI']

        return v3d(pt=pt, eta=et, phi=ph)
    # ---------------------------------
    def _get_correction(self, row : pnd.Series, lepton : str) -> float:
        norm = self._get_normal(row=row)
        l1_p = self._get_momentum(row=row, name='L1')
        l2_p = self._get_momentum(row=row, name='L2')
        kp_p = self._get_momentum(row=row, name= 'H')

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
    def _build_features(self, row_sr : pnd.Series) -> pnd.Series:
        lep = 'L1' if row_sr['L1_brem'] == 1 else 'L2'

        data        = {}
        data['row'] = row_sr[f'{lep}_BREMHYPOROW']
        data['col'] = row_sr[f'{lep}_BREMHYPOCOL']
        data['are'] = row_sr[f'{lep}_BREMHYPOAREA']
        data['eng'] = row_sr[f'{lep}_BREMTRACKBASEDENERGY'] / 1000 # Very large numbers seem to break down training
        data['npv'] = row_sr['nPVs']
        data['per'] = row_sr['period']
        data['blk'] = row_sr['block']
        data['mu' ] = 1000 * self._get_correction(row=row_sr, lepton=lep)

        row_sr = pnd.Series(data)

        return row_sr
    # ---------------------------------
    def _values(self, kind : str) -> tensor:
        ddf = self.get_data()
        df  = ddf.compute()

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

        ddf = ddf.apply(self._build_features, axis=1)
        ddf = ddf.persist()

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
# --------------------------
