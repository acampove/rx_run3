'''
Module storing MassBiasCorrector class
'''
# pylint: disable=too-many-return-statements

import vector
import numpy
import pandas as pnd
from pandarallel                     import pandarallel
from ROOT                            import RDataFrame, RDF
from dmu.logging.log_store           import LogStore

import rx_data.utilities             as ut
from rx_data.electron_bias_corrector import ElectronBiasCorrector

log=LogStore.add_logger('rx_data:mass_bias_corrector')
# ------------------------------------------
class MassBiasCorrector:
    '''
    Class meant to correct B mass without DTF constraint
    by correcting biases in electrons
    '''
    # ------------------------------------------
    def __init__(self,
                 rdf                   : RDataFrame,
                 skip_correction       : bool  = False,
                 nthreads              : int   = 1,
                 brem_energy_threshold : float = 400,
                 ecorr_kind            : str   = 'brem_track_2'):
        '''
        rdf : ROOT dataframe
        skip_correction: Will do everything but not correction. Needed to check that only the correction is changing data.
        nthreads : Number of threads, used by pandarallel
        brem_energy_threshold: Lowest energy that an ECAL cluster needs to have to be considered a photon, used as argument of ElectronBiasCorrector, default 0 (MeV)
        ecorr_kind : Kind of correction to be added to electrons, [ecalo_bias, brem_track]
        '''
        self._df              = ut.df_from_rdf(rdf)
        self._skip_correction = skip_correction
        self._nthreads        = nthreads

        self._ebc        = ElectronBiasCorrector(brem_energy_threshold = brem_energy_threshold)
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
    def _calculate_variables(self, row : pnd.Series) -> float:
        l1 = vector.obj(pt=row.L1_PT, phi=row.L1_PHI, eta=row.L1_ETA, m=self._emass)
        l2 = vector.obj(pt=row.L2_PT, phi=row.L2_PHI, eta=row.L2_ETA, m=self._emass)
        kp = vector.obj(pt=row.H_PT , phi=row.H_PHI , eta=row.H_ETA , m=self._kmass)

        jp = l1 + l2
        bp = jp + kp

        bmass = -1 if numpy.isnan(bp.mass) else float(bp.mass)
        jmass = -1 if numpy.isnan(jp.mass) else float(jp.mass)

        d_data = {
                'B_M'    : bmass,
                'Jpsi_M' : jmass,
                # --------------
                'L1_PX'  : row.L1_PX,
                'L1_PY'  : row.L1_PY,
                'L1_PZ'  : row.L1_PZ,
                'L1_PT'  : row.L1_PT,
                # --------------
                'L2_PX'  : row.L2_PX,
                'L2_PY'  : row.L2_PY,
                'L2_PZ'  : row.L2_PZ,
                'L2_PT'  : row.L2_PT,
                # --------------
                'L1_HASBREMADDED' : row.L1_HASBREMADDED,
                'L2_HASBREMADDED' : row.L2_HASBREMADDED,
                }

        df = pnd.Series(d_data)

        return df
    # ------------------------------------------
    def _calculate_correction(self, row : pnd.Series) -> pnd.DataFrame:
        row  = self._correct_electron('L1', row)
        row  = self._correct_electron('L2', row)
        df   = self._calculate_variables(row)

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
        if suffix is None:
            return df

        df = df.add_suffix(f'_{suffix}')

        return df
    # ------------------------------------------
    def get_rdf(self, suffix: str = None) -> RDataFrame:
        '''
        Returns corrected ROOT dataframe

        mass_name (str) : Name of the column containing the corrected mass, by default B_M
        '''
        log.info('Applying bias correction')

        df = self._df
        if self._nthreads > 1:
            sr_data = df.parallel_apply(self._calculate_correction, axis=1)
        else:
            sr_data = df.apply(self._calculate_correction, axis=1)

        l_var     = sr_data.columns
        df[l_var] = sr_data

        df        = self._filter_df(df)
        df        = df.fillna(-1)
        df        = self._add_suffix(df, suffix)
        rdf       = RDF.FromPandas(df)

        return rdf
# ------------------------------------------
