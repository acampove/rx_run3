'''
Module containing Q2SmearCalculator class
'''
import os
import pandas as pnd
from dmu.logging.log_store          import LogStore
from dmu.generic.version_management import get_last_version

log = LogStore.add_logger('rx_q2:q2smear_corrector')
# ------------------------------------
class Q2SmearCorrector:
    '''
    Class intended to operate only on simulated datasets from the electron channel to:

    - Read mass scales and resolutions
    - Calculated smeared masses
    - Returns smeared mass for each unsmeared mass, block and brem 
    '''
    # ------------------------------------
    def __init__(self):
        '''
        No arguments needed, inputs will be passed to `get_mass`
        '''
        self._jpsi_pdg_mass = 3096.9
        log.debug(f'Using Jpsi PDG mass: {self._jpsi_pdg_mass:.2f}')

        self._df = self._get_scales()
    # ------------------------------------
    def _get_scales(self) -> pnd.DataFrame:
        ana_dir = os.environ['ANADIR']
        par_dir = get_last_version(f'{ana_dir}/q2/fits', version_only=False)
        par_path= f'{par_dir}/parameters.json'
        log.info(f'Reading scales from: {par_path}')
        df      = pnd.read_json(par_path)

        return df
    # ------------------------------------
    def _read_quantity(self, nbrem : int, block : int, kind : str) -> float:
        df   = self._df
        if kind == 'mu_mc':
            mu_mc = df.loc[ (df['block'] == block) & (df['brem'] == nbrem) & (df['sample'] == 'sim'), 'mu_val' ].iloc[0]
            return mu_mc

        if kind == 'reso':
            sg_dt = df.loc[ (df['block'] == block) & (df['brem'] == nbrem) & (df['sample'] == 'dat'), 'sg_val' ].iloc[0]
            sg_mc = df.loc[ (df['block'] == block) & (df['brem'] == nbrem) & (df['sample'] == 'sim'), 'sg_val' ].iloc[0]
            return sg_dt / sg_mc

        if kind == 'scale':
            mu_dt = df.loc[ (df['block'] == block) & (df['brem'] == nbrem) & (df['sample'] == 'dat'), 'mu_val' ].iloc[0]
            mu_mc = df.loc[ (df['block'] == block) & (df['brem'] == nbrem) & (df['sample'] == 'sim'), 'mu_val' ].iloc[0]

            return mu_dt - mu_mc

        raise NotImplementedError(f'Invalid quantity: {kind}')
    # ------------------------------------
    def get_mass(
            self,
            nbrem          : int,
            block          : int,
            jpsi_mass_true : float,
            jpsi_mass_reco : float) -> float:
        '''
        Parameters:
        ---------------
        brem          : Integer with the brem category 0, 1, 2 
        block         : Integer with the block [0-8] 
        jpsi_mass_true: True mass of Jpsi
        jpsi_mass_reco: Value of unsmeared Jpsi mass

        Returns:
        ---------------
        Smeared mass
        '''
        mu_mc = self._read_quantity(nbrem=nbrem, block=block, kind='mu_mc')
        reso  = self._read_quantity(nbrem=nbrem, block=block, kind= 'reso')
        scale = self._read_quantity(nbrem=nbrem, block=block, kind='scale')
        mass  = jpsi_mass_true + reso * (jpsi_mass_reco - jpsi_mass_true) + scale + (1 - reso) * (mu_mc - self._jpsi_pdg_mass)

        log.debug(f'{jpsi_mass_reco:20.0f}{"->:<20"}{mass:<20.0f}')

        return mass
# ------------------------------------
