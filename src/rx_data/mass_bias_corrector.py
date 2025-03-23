'''
Module storing MassBiasCorrector class
'''
# pylint: disable=too-many-return-statements

import math

import vector
import pandas as pnd
from pandarallel                     import pandarallel
from ROOT                            import RDataFrame, RDF
from dmu.logging.log_store           import LogStore
from rx_data.electron_bias_corrector import ElectronBiasCorrector

log=LogStore.add_logger('rx_data:mass_bias_corrector')
# ------------------------------------------
def df_from_rdf(rdf : RDataFrame) -> pnd.DataFrame:
    '''
    Utility method needed to get pandas dataframe from ROOT dataframe
    '''
    rdf    = _preprocess_rdf(rdf)
    l_col  = [ name.c_str() for name in rdf.GetColumnNames() if _pick_column(name.c_str()) ]
    d_data = rdf.AsNumpy(l_col)
    df     = pnd.DataFrame(d_data)

    return df
# ------------------------------------------
def _preprocess_rdf(rdf: RDataFrame) -> RDataFrame:
    rdf = rdf.Redefine('L1_HASBREMADDED', 'int(L1_HASBREMADDED)')
    rdf = rdf.Redefine('L2_HASBREMADDED', 'int(L2_HASBREMADDED)')

    return rdf
# ------------------------------------------
def _pick_column(name : str) -> bool:
    to_keep  = ['EVENTNUMBER', 'RUNNUMBER']

    if name in to_keep:
        return True

    not_l1 = not name.startswith('L1')
    not_l2 = not name.startswith('L2')
    not_kp = not name.startswith('H')

    if not_l1 and not_l2 and not_kp:
        return False

    if 'BREMTRACKBASEDENERGY' in name:
        return True

    if 'HASBREMADDED' in name:
        return True

    if 'NVPHITS' in name:
        return False

    if 'CHI2' in name:
        return False

    if 'HYPOID' in name:
        return False

    if 'HYPODELTA' in name:
        return False

    if 'PT' in name:
        return True

    if 'ETA' in name:
        return True

    if 'PHI' in name:
        return True

    if 'PX' in name:
        return True

    if 'PY' in name:
        return True

    if 'PZ' in name:
        return True

    if 'BREMHYPO' in name:
        return True

    return False
# ------------------------------------------
class MassBiasCorrector:
    '''
    Class meant to correct B mass without DTF constraint
    by correcting biases in electrons
    '''
    # ------------------------------------------
    def __init__(self,
                 rdf             : RDataFrame,
                 skip_correction : bool = False,
                 nthreads        : int  = 1,
                 ecorr_kind      : str  = 'brem_track'):
        '''
        rdf : ROOT dataframe
        skip_correction: Will do everything but not correction. Needed to check that only the correction is changing data.
        nthreads : Number of threads, used by pandarallel
        ecorr_kind : Kind of correction to be added to electrons, [ecalo_bias, brem_track]
        '''
        self._df              = df_from_rdf(rdf)
        self._skip_correction = skip_correction
        self._nthreads        = nthreads

        self._ebc        = ElectronBiasCorrector()
        self._emass      = 0.511
        self._kmass      = 493.6
        self._ecorr_kind = ecorr_kind

        self._set_loggers()

        if self._nthreads > 1:
            pandarallel.initialize(nb_workers=self._nthreads, progress_bar=True)
    # ------------------------------------------
    def _set_loggers(self) -> None:
        LogStore.set_level('rx_data:brem_bias_corrector'    , 50)
        LogStore.set_level('rx_data:electron_bias_corrector', 50)
    # ------------------------------------------
    def _correct_electron(self, name : str, row : pnd.Series) -> pnd.Series:
        if self._skip_correction:
            return row

        row = self._ebc.correct(row, name=name, kind=self._ecorr_kind)

        return row
    # ------------------------------------------
    def _calculate_masses(self, row : pnd.Series) -> float:
        l1 = vector.obj(pt=row.L1_PT, phi=row.L1_PHI, eta=row.L1_ETA, m=self._emass)
        l2 = vector.obj(pt=row.L2_PT, phi=row.L2_PHI, eta=row.L2_ETA, m=self._emass)
        kp = vector.obj(pt=row.H_PT , phi=row.H_PHI , eta=row.H_ETA , m=self._kmass)

        jp = l1 + l2
        bp = jp + kp

        bmass = float(bp.mass) if float(bp.mass) else -1
        jmass = float(jp.mass) if float(jp.mass) else -1

        return pnd.Series({'B_M' : bmass, 'Jpsi_M' : jmass})
    # ------------------------------------------
    def _calculate_correction(self, row : pnd.Series) -> pnd.DataFrame:
        row  = self._correct_electron('L1', row)
        row  = self._correct_electron('L2', row)
        df   = self._calculate_masses(row)

        return df
    # ------------------------------------------
    def _filter_df(self, df : pnd.DataFrame) -> float:
        l_to_keep  = ['L1_PT', 'L1_PX', 'L1_PY', 'L1_PZ', 'L1_HASBREMADDED']
        l_to_keep += ['L2_PT', 'L2_PX', 'L2_PY', 'L2_PZ', 'L2_HASBREMADDED']
        l_to_keep += ['B_M'  , 'Jpsi_M', 'EVENTNUMBER', 'RUNNUMBER']

        log.debug(20 * '-')
        log.debug('Keeping variables:')
        log.debug(20 * '-')
        for name in l_to_keep:
            log.debug(f'    {name}')

        df = df[l_to_keep]

        return df
    # ------------------------------------------
    def _add_suffix(self, df : pnd.DataFrame, suffix : str):
        return df
    # ------------------------------------------
    def get_rdf(self, suffix: str = 'corr') -> RDataFrame:
        '''
        Returns corrected ROOT dataframe

        mass_name (str) : Name of the column containing the corrected mass, by default B_M
        '''
        log.info('Applying bias correction')

        df        = self._df
        if self._nthreads > 1:
            df[['B_M', 'Jpsi_M']] = df.parallel_apply(self._calculate_correction, axis=1)
        else:
            df[['B_M', 'Jpsi_M']] = df.apply(self._calculate_correction, axis=1)

        df        = self._filter_df(df)
        df        = df.fillna(-1)
        df        = self._add_suffix(df, suffix)
        rdf       = RDF.FromPandas(df)

        return rdf
# ------------------------------------------
