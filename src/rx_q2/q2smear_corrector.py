'''
Module containing Q2SmearCalculator class
'''

import os
from typing    import Final
from functools import cache

import pandas as pnd
from ROOT                           import RDF
from dmu.generic                    import typing_utilities as tut
from dmu.logging.log_store          import LogStore
from dmu.generic.version_management import get_last_version

log = LogStore.add_logger('rx_q2:q2smear_corrector')

JPSI_PDG_MASS : Final[float] = 3096.9
# ------------------------------------
class Q2SmearCorrector:
    '''
    Class intended to operate only on simulated datasets from the electron or muon channel to:

    - Read mass scales and resolutions
    - Calculate smeared and scaled masses
    - Returns smeared mass for each unsmeared mass, block and brem 
    '''
    # ------------------------------------
    def __init__(self, channel : str):
        '''
        No arguments needed, inputs will be passed to `get_mass`

        Parameters
        -------------------
        channel: E.g. ee or mm, needed to pick up the JSON file with scales
        '''
        log.debug(f'Using Jpsi PDG mass: {JPSI_PDG_MASS:.2f}')

        self._channel = channel
        self._df      = self._get_scales()
    # ------------------------------------
    def _get_scales(self) -> pnd.DataFrame:
        ana_dir = os.environ['ANADIR']
        par_dir = get_last_version(f'{ana_dir}/q2/fits', version_only=False)
        par_path= f'{par_dir}/parameters_{self._channel}.json'

        log.info(f'Reading scales from: {par_path}')
        df      = pnd.read_json(par_path)

        return df
    # ------------------------------------
    @cache
    def _read_quantity(self, nbrem : int, block : int, kind : str) -> float:
        df = self._df.query(f'block == {block} and brem == {nbrem}')

        if len(df) != 2:
            log.info(df)
            raise ValueError(f'Expected data and MC entries for brem/block: {nbrem}/{block}')

        if kind == 'mu_mc':
            mu_mc = df.loc[ (df['sample'] == 'sim'), 'mu_val' ].iloc[0]
            return mu_mc

        if kind == 'reso':
            sg_dt = df.loc[ (df['sample'] == 'dat'), 'sg_val' ].iloc[0]
            sg_mc = df.loc[ (df['sample'] == 'sim'), 'sg_val' ].iloc[0]
            return sg_dt / sg_mc

        if kind == 'scale':
            mu_dt = df.loc[ (df['sample'] == 'dat'), 'mu_val' ].iloc[0]
            mu_mc = df.loc[ (df['sample'] == 'sim'), 'mu_val' ].iloc[0]

            return mu_dt - mu_mc

        raise NotImplementedError(f'Invalid quantity: {kind}')
    # ------------------------------------
    def _smear_mass(
        self,
        row      : pnd.Series,
        particle : str) -> float:
        '''
        Parameters:
        ---------------
        row : DataFrame row representing candiate 

        Returns:
        ---------------
        Smeared mass
        '''
        block = tut.numeric_from_series(row=row, name='block', numeric=int)
        nbrem = tut.numeric_from_series(row=row, name='nbrem', numeric=int)
        recom = tut.numeric_from_series(row=row, name=f'{particle}_M_brem_track_2', numeric=float)
        truem = tut.numeric_from_series(row=row, name=f'{particle}_TRUEID'        , numeric=float)

        mu_mc = self._read_quantity(nbrem=nbrem, block=block, kind='mu_mc')
        reso  = self._read_quantity(nbrem=nbrem, block=block, kind= 'reso')
        scale = self._read_quantity(nbrem=nbrem, block=block, kind='scale')

        mass  = truem + reso * (recom - truem) + scale + (1 - reso) * (mu_mc - JPSI_PDG_MASS)

        log.debug(f'{recom:20.0f}{"->:<20"}{mass:<20.0f}')

        return mass
    # ----------------------
    def get_rdf(self, rdf : RDF.RNode) -> RDF.RNode:
        '''
        Parameters
        -------------
        rdf: ROOT dataframe with mass columns to be smeared

        Returns
        -------------
        ROOT dataframe with data to be smeared 
        '''
        columns = ['B_M_brem_track_2', 'Jpsi_M_brem_track_2', 
                   'B_TRUEID'        , 'Jpsi_TRUEID',
                   'nbrem'           , 'block']
        data    = rdf.AsNumpy(columns)
        df      = pnd.DataFrame(data)

        for particle in ['B', 'Jpsi']:
            df[f'{particle}_Mass'] = df.apply(self._smear_mass, args=(particle,), axis=1)

        return RDF.FromPandas(df) 
# ------------------------------------
