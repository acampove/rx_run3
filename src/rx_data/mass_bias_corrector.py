'''
Module storing MassBiasCorrector class
'''
# pylint: disable=too-many-return-statements

import math
import vector
import pandas as pnd

from ROOT                            import RDataFrame, RDF
from dmu.logging.log_store           import LogStore
from rx_data.electron_bias_corrector import ElectronBiasCorrector

log=LogStore.add_logger('rx_data:mass_bias_corrector')
# ------------------------------------------
class MassBiasCorrector:
    '''
    Class meant to correct B mass without DTF constraint
    by correcting biases in electrons
    '''
    # ------------------------------------------
    def __init__(self, rdf : RDataFrame, skip_correction : bool = False):
        self._rdf             = self._preprocess_rdf(rdf)
        self._skip_correction = skip_correction

        self._ebc     = ElectronBiasCorrector()
        self._emass   = 0.511
        self._kmass   = 493.6
        self._to_keep = ['EVENTNUMBER', 'RUNNUMBER', 'B_M']
    # ------------------------------------------
    def _preprocess_rdf(self, rdf: RDataFrame) -> RDataFrame:
        rdf = rdf.Redefine('L1_HASBREMADDED', 'int(L1_HASBREMADDED)')
        rdf = rdf.Redefine('L2_HASBREMADDED', 'int(L2_HASBREMADDED)')

        return rdf
    # ------------------------------------------
    def _correct_electron(self, name : str, row : pnd.Series) -> pnd.Series:
        if self._skip_correction:
            return row

        row = self._ebc.correct(row, name=name)

        return row
    # ------------------------------------------
    def _calculate_mass(self, row : pnd.Series) -> float:
        l1 = vector.obj(pt=row.L1_PT, phi=row.L1_PHI, eta=row.L1_ETA, m=self._emass)
        l2 = vector.obj(pt=row.L2_PT, phi=row.L2_PHI, eta=row.L2_ETA, m=self._emass)
        kp = vector.obj(pt=row.H_PT , phi=row.H_PHI , eta=row.H_ETA , m=self._kmass)
        bp = l1 + l2 + kp

        mass = float(bp.mass)
        if math.isnan(mass):
            log.warning('NaN mass found for:')
            log.info(f'L1: {l1}')
            log.info(f'L2: {l2}')
            log.info(f'Kp: {kp}')
            log.info(f'Bp: {bp}')

        return mass
    # ------------------------------------------
    def _calculate_correction(self, row : pnd.Series) -> float:
        row  = self._correct_electron('L1', row)
        row  = self._correct_electron('L2', row)
        mass = self._calculate_mass(row)

        return mass
    # ------------------------------------------
    def _pick_column(self, name : str) -> bool:
        col_type = self._rdf.GetColumnType(name)
        if 'RVec' in col_type:
            return False

        if col_type == 'Bool_t':
            return False

        if 'Hlt' in name:
            return False

        if 'DTF' in name:
            return False

        if name in self._to_keep:
            return True

        if name.startswith('H_'):
            #log.info(f'{col_type:<20}{name}')
            return True

        if name.startswith('L1_'):
            #log.info(f'{col_type:<20}{name}')
            return True

        if name.startswith('L2_'):
            #log.info(f'{col_type:<20}{name}')
            return True

        return False
    # ------------------------------------------
    def _df_from_rdf(self):
        l_col  = [ name.c_str() for name in self._rdf.GetColumnNames() if self._pick_column(name.c_str()) ]
        d_data = self._rdf.AsNumpy(l_col)
        df     = pnd.DataFrame(d_data)

        return df
    # ------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns corrected ROOT dataframe
        '''
        log.info('Applying bias correction')

        df             = self._df_from_rdf()
        df['B_M_corr'] = df.apply(self._calculate_correction, axis=1)

        df  = df[self._to_keep + ['B_M_corr']]
        rdf = RDF.FromPandas(df)

        return rdf
# ------------------------------------------
