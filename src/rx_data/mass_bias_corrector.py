'''
Module storing MassBiasCorrector class
'''

from typing import cast, Final

import numpy
from dask             import dataframe as dd 

import pandas as pnd
import vector
from vector                          import MomentumObject3D as v3d
from vector                          import MomentumObject4D as v4d
from dmu.logging.log_store           import LogStore
from rx_q2.q2smear_corrector         import Q2SmearCorrector
from dmu.generic                     import typing_utilities as tut

from rx_common                       import info
from rx_data.electron_bias_corrector import ElectronBiasCorrector

log=LogStore.add_logger('rx_data:mass_bias_corrector')

EMASS  :Final[float] = 0.511
KMASS  :Final[float] = 493.6
PIMASS :Final[float] = 137.57
# ------------------------------------------
class MassBiasCorrector:
    '''
    Class meant to correct B mass without DTF constraint
    by correcting biases in electrons due to:

    - Issues with brem recovery: For this we use the `ElectronBiasCorrector` with `brem_track_2` correction
    - Differences in scale and resolution: For this we use the `Q2SmearCorrector`
    '''
    # ------------------------------------------
    def __init__(
        self,
        df                    : pnd.DataFrame,
        is_mc                 : bool,
        trigger               : str,
        skip_correction       : bool  = False,
        nthreads              : int   = 1,
        brem_energy_threshold : float = 400,
        ecorr_kind            : str   = 'brem_track_2'):
        '''
        Parameters
        --------------
        rdf                  : ROOT dataframe
        trigger              : Hlt2 trigger name
        skip_correction      : Will do everything but not correction. Needed to check that only the correction is changing data.
        nthreads             : Number of processes to use 
        brem_energy_threshold: Lowest energy that an ECAL cluster needs to have to be considered a photon, used as argument of ElectronBiasCorrector, default 0 (MeV)
        ecorr_kind           : Kind of correction to be added to electrons, [ecalo_bias, brem_track]
        '''
        self._df              = df
        self._is_mc           = is_mc 
        self._trigger         = trigger
        self._skip_correction = skip_correction
        self._nproc           = nthreads

        self._ebc             = ElectronBiasCorrector(brem_energy_threshold = brem_energy_threshold)
        self._ecorr_kind      = ecorr_kind

        self._qsq_corr   = Q2SmearCorrector()
        self._project : Final[str] = info.project_from_trigger(trigger=self._trigger, lower_case=True)

        log.info(f'Using project: {self._project}')

        self._silence_logger(name = 'rx_data:brem_bias_corrector')
        self._silence_logger(name = 'rx_data:electron_bias_corrector')
    # ------------------------------------------
    def _silence_logger(self, name) -> None:
        logger = LogStore.get_logger(name=name)
        if logger is None:
            raise ValueError(f'Cannot get logger: {name}')

        # If a logger has been put in debug level
        # then it is not meant to be silenced here
        if logger.getEffectiveLevel() == 10:
            return

        LogStore.set_level(name, 50)
    # ------------------------------------------
    def _correct_electron(
        self, 
        name : str, 
        row  : pnd.Series) -> pnd.Series:
        '''
        Parameters
        -------------------
        name: Name of particle, e.g. L1
        row : Row in pandas dataframe
        '''
        if self._skip_correction:
            log.debug('Skipping correction for {name}')
            return row

        row = self._ebc.correct(row, name=name, kind=self._ecorr_kind)

        return row
    # ----------------------
    def _build_4dvec(self, particle : str, row : pnd.Series, mass : float) -> v4d:
        '''
        Parameters
        -------------
        particle: Particle name, e.g. L1
        row     : Pandas series with event information
        mass    : Mass of particle

        Returns
        -------------
        Lorentz vector for particle
        '''
        pt = tut.numeric_from_series(row, f'{particle}_PT' , float)
        eta= tut.numeric_from_series(row, f'{particle}_ETA', float)
        phi= tut.numeric_from_series(row, f'{particle}_PHI', float)

        return vector.obj(pt=pt, phi=phi, eta=eta, mass=mass)
    # ------------------------------------------
    def _calculate_variables(self, row : pnd.Series) -> pnd.Series:
        '''
        Parameters
        ----------------
        row: Series representing event

        Returns 
        ----------------
        Series with recalculated kinematics, after corrections
        '''
        l1 = self._build_4dvec(particle='L1', row=row, mass=EMASS)
        l2 = self._build_4dvec(particle='L2', row=row, mass=EMASS)

        if   self._project == 'rk':
            hd = self._build_4dvec(particle= 'H', row=row, mass=KMASS)
        elif self._project == 'rkst':
            h1 = self._build_4dvec(particle='H1', row=row, mass=KMASS)
            h2 = self._build_4dvec(particle='H2', row=row, mass=PIMASS)

            hd = h1 + h2
        else:
            raise ValueError(f'Invalid project: {self._project}')

        jp = l1 + l2
        bp = jp + hd

        jp = cast(v4d, jp)
        bp = cast(v4d, bp)

        bmass = bp.mass
        jmass = jp.mass

        bmass = -1 if numpy.isnan(bmass) else float(bmass)
        jmass = -1 if numpy.isnan(jmass) else float(jmass)

        # TODO: Needs to recalculate:
        # PIDe
        # ProbNNe
        d_data = {
                'B_M'    : bmass,
                'Jpsi_M' : jmass,
                # --------------
                'B_PT'   : bp.pt,
                'Jpsi_PT': jp.pt,
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

        d_data['Jpsi_M_smr'] = self._smear_mass(row, particle='Jpsi', reco=jmass)
        d_data[   'B_M_smr'] = self._smear_mass(row, particle=   'B', reco=bmass)

        d_data[   'B_DIRA_OWNPV'] = self._calculate_dira(momentum=bp.to_Vector3D(), row=row, particle=   'B')
        d_data['Jpsi_DIRA_OWNPV'] = self._calculate_dira(momentum=jp.to_Vector3D(), row=row, particle='Jpsi')

        sr = pnd.Series(d_data)

        return sr
    # ------------------------------------------
    def _calculate_dira(
        self,
        row      : pnd.Series,
        momentum : v3d,
        particle : str) -> float:
        '''
        Recalculates dira with brem corrected momentum

        Parameters
        ----------------
        row      : Series associated to candidate information
        momentum : Brem corrected momentum
        particle : String with name of particle, e.g. B, Jpsi
        '''
        pv_x = row[f'{particle}_BPVX']
        pv_y = row[f'{particle}_BPVY']
        pv_z = row[f'{particle}_BPVZ']

        sv_x = row[f'{particle}_END_VX']
        sv_y = row[f'{particle}_END_VY']
        sv_z = row[f'{particle}_END_VZ']

        pv   = v3d(x=pv_x, y=pv_y, z=pv_z)
        sv   = v3d(x=sv_x, y=sv_y, z=sv_z)
        dr   = sv - pv
        dr   = cast(v3d, dr)

        cos_theta = dr.dot(momentum) / (dr.mag * momentum.mag)

        return cos_theta
    # ------------------------------------------
    def _smear_mass(self, row : pnd.Series, particle : str, reco : float) -> float:
        if not self._is_mc:
            return reco

        true    = row[f'{particle}_TRUEM']
        nbrem   = row['L1_HASBREMADDED'] + row['L2_HASBREMADDED']
        block   = row['block']
        smeared = self._qsq_corr.get_mass(nbrem=nbrem, block=block, jpsi_mass_reco=reco, jpsi_mass_true=true)

        return smeared
    # ------------------------------------------
    def _calculate_correction(self, row : pnd.Series) -> pnd.Series:
        row  = self._correct_electron('L1', row)
        row  = self._correct_electron('L2', row)

        # NOTE: The variable calculation has to be done on the row AFTER the correction
        row  = self._calculate_variables(row)

        return row
    # ------------------------------------------
    def _add_suffix(self, df : pnd.DataFrame, suffix : str|None):
        if suffix is None:
            return df

        df = df.add_suffix(f'_{suffix}')

        return df
    # ----------------------
    def _get_corrected_df(self) -> pnd.DataFrame:
        '''
        Returns
        -------------
        Dataframe after correction
        '''
        if self._nproc == 1:
            log.info('Using single process to correct data')
            return self._df.apply(self._calculate_correction, axis=1)

        log.info(f'Using {self._nproc} processes to correct data')
        ddf = dd.from_pandas(self._df, npartitions=self._nproc)
        df  = ddf.map_partitions(lambda x : x.apply(self._calculate_correction, axis=1)).compute()

        return df
    # ------------------------------------------
    def get_df(self, suffix: str|None = None) -> pnd.DataFrame:
        '''
        Returns corrected pandas dataframe

        mass_name (str) : Name of the column containing the corrected mass, by default B_M
        '''
        log.info('Applying bias correction')

        df_corr = self._get_corrected_df()
        df_corr = self._add_suffix(df_corr, suffix)

        for variable in ['EVENTNUMBER', 'RUNNUMBER']:
            df_corr[variable] = self._df[variable]

        df_corr = df_corr.fillna(-1) # For some candidates the B mass after correction becomes NaN

        in_size = len(df_corr)
        ot_size = len(self._df)
        if in_size != ot_size:
            raise ValueError(f'Sizes of input and output dataframes differ, {in_size} != {ot_size}')

        return df_corr 
# ------------------------------------------
