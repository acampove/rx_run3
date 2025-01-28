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
        '''
        rdf_gen: ROOT dataframe with generator level candidates (MCDecayTree), by default None
        rdf_rec: ROOT dataframe with reconstructed candidates (DecayTree)

        Two modes are implemented:

        - Only `rdf_rec` is passed: Then the class only assigns columns to this dataframe.
        - Both dataframes are passed: Then the reco tree is used to add columns to the `rdf_gen` dataframe.
        '''
        self._rdf_rec = rdf_rec
        self._rdf_gen = rdf_gen
    # ---------------------------
    def _add_to_rec(self):
        return self._rdf_rec
    # ---------------------------
    def _add_to_gen(self):
        return self._rdf_gen
    # ---------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe after adding column
        '''

        if self._rdf_gen is None:
            rdf = self._add_to_gen()
        else:
            rdf = self._add_to_rec()

        return rdf
# -----------------------------
