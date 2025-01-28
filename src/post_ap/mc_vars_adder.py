'''
Module holding MCVarsAdder class
'''
from typing import Union

from ROOT import RDataFrame

# -----------------------------
class MCVarsAdder:
    '''
    Class intended to add columns to ROOT dataframe representing MC
    '''
    # ---------------------------
    def __init__(self, rdf_rec : RDataFrame, rdf_gen : Union[RDataFrame,None] = None):
        self._rdf_rec = rdf_rec
        self._rdf_gen = rdf_gen
    # ---------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe after adding column
        '''
        return self._rdf_rec
# -----------------------------
