'''
Module containing MassCalculator class
'''
from typing import cast

import pandas as pnd
from ROOT                  import RDataFrame, RDF
from particle              import Particle         as part
from vector                import MomentumObject4D as v4d
from dmu.generic           import typing_utilities as tut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:mass_calculator')
# ---------------------------
class MassCalculator:
    '''
    Class in charge of creating dataframe with extra mass branches
    These are meant to be different from the Swap branches because
    the full candidate is meant to be rebuilt with different mass
    hypotheses for the tracks
    '''
    # ----------------------
    def __init__(self, rdf : RDataFrame|RDF.RNode) -> None:
        '''
        Parameters
        -------------
        rdf : ROOT dataframe
        '''
        self._rdf = rdf
    # ----------------------
    def _get_columns(self, row) -> pnd.Series:
        '''
        Returns
        -------------
        Row of pandas dataframe with masses
        '''
        evt = tut.numeric_from_series(row, 'EVENTNUMBER', int)
        run = tut.numeric_from_series(row, 'RUNNUMBER'  , int)

        out = pnd.Series({'EVENTNUMBER' : evt, 'RUNNUMBER' : run})

        return out
    # ----------------------
    def _get_hxy_mass(
        self,
        row : pnd.Series,
        x   : int,
        y   : int) -> float:
        '''
        Parameters
        -------------
        row: Series with event information
        x/y: PDG ID to replace L1/L2 lepton with

        Returns
        -------------
        Value of mass when leptons get pion, hadron, etc mass hypothesis
        '''
        had_4d = self._get_hadronic_system_4d(row=row)
        par_1  = self._build_particle(row=row, name='L1', pid=x)
        par_2  = self._build_particle(row=row, name='L2', pid=y)

        candidate = had_4d + par_1 + par_2
        candidate = cast(v4d, candidate)

        return candidate.mass
    # ----------------------
    def _build_particle(
        self,
        row  : pnd.Series,
        name : str,
        pid  : int) -> v4d:
        '''
        Parameters
        -------------
        row  : Pandas series with event information
        name : Name of particle in original ROOT dataframe, e.g. L1
        pid  : PDG ID of particle that needs to be built, e.g. 11 for electron
              Will be used to get mass.

        Returns
        -------------
        Particle with the kinematics in the original dataframe
        but with the mass hypothesis corresponding to
        '''
        mass     = self._mass_from_pid(pid=pid)
        particle = self._get_particle(row=row, name=name)

        return v4d(pt=particle.pt, eta=particle.eta, phi=particle.phi, mass=mass)
    # ----------------------
    def _get_hadronic_system_4d(self, row : pnd.Series) -> v4d:
        '''
        Parameters
        -------------
        row: Pandas series with event information

        Returns
        -------------
        Four momentum vector of hadronic system
        '''
        b_4d  = self._get_particle(row=row, name='B')
        l1_4d = self._get_particle(row=row, name='L1')
        l2_4d = self._get_particle(row=row, name='L2')

        res   = b_4d - l1_4d - l2_4d
        res   = cast(v4d, res)

        return res
    # ----------------------
    def _get_particle(self, row : pnd.Series, name : str) -> v4d:
        '''
        Parameters
        -------------
        row: Pandas series with event information
        name: Name of particle whose 4D vector to extract

        Returns
        -------------
        4D vector for particle
        '''
        particle_id = tut.numeric_from_series(row, f'{name}_ID' ,   int)
        pt = tut.numeric_from_series(row, f'{name}_PT' , float)
        et = tut.numeric_from_series(row, f'{name}_ETA', float)
        ph = tut.numeric_from_series(row, f'{name}_PHI', float)
        mass = self._mass_from_pid(pid=particle_id)

        return v4d(pt=pt, eta=et, phi=ph, mass=mass)
    # ----------------------
    def _mass_from_pid(self, pid : int) -> float:
        '''
        Parameters
        -------------
        pid: Particle PDG ID

        Returns
        -------------
        Mass of particle
        '''
        particle = part.from_pdgid(pid)
        mass     = particle.mass
        if mass is None:
            raise ValueError(f'Cannot find mass of particle with ID: {pid}')

        return mass
    # ----------------------
    def _is_valid_column(self, name : str) -> bool:
        '''
        Parameters
        -------------
        name: Name of column in ROOT dataframe

        Returns
        -------------
        True or False, depending on wether this column is needed
        '''
        if name in ['EVENTNUMBER', 'RUNNUMBER', 'B_M', 'B_PT', 'B_ETA', 'B_PHI']:
            return True

        if name in ['L1_TRACK_PT', 'L1_TRACK_ETA', 'L1_TRACK_PHI']:
            return True

        if name in ['L2_TRACK_PT', 'L2_TRACK_ETA', 'L2_TRACK_PHI']:
            return True

        # Need the original masses
        if name in ['B_ID', 'L1_ID', 'L2_ID']:
            return True

        return False
    # ----------------------
    def _get_dataframe(self) -> pnd.DataFrame:
        '''
        Returns
        -------------
        pandas dataframe with only necessary information
        '''
        l_col = [ name.c_str() for name in self._rdf.GetColumnNames() ]
        l_col = [ name         for name in l_col if self._is_valid_column(name=name) ]

        data  = self._rdf.AsNumpy(l_col)
        df    = pnd.DataFrame(data)

        return df
    # ----------------------
    def get_rdf(self) -> RDataFrame|RDF.RNode:
        '''
        Returns
        -------------
        ROOT dataframe with only the new mass columns
        EVENTNUMBER and RUNNUMBER
        '''
        df  = self._get_dataframe()
        df  = df.apply(self._get_columns, axis=1)
        df  = cast(pnd.DataFrame, df)
        data= { col_name : df[col_name].to_numpy() for col_name in df.columns }
        rdf = RDF.FromNumpy(data)

        return rdf
# ---------------------------
