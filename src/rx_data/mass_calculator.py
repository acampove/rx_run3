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
