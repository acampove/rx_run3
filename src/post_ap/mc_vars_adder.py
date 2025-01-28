'''
Module holding MCVarsAdder class
'''
import re
from typing import Union

from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('post_ap:mc_vars_adder')
# -----------------------------
class MCVarsAdder:
    '''
    Class intended to add columns to ROOT dataframe representing MC
    '''
    # ---------------------------
    def __init__(self,
                 sample_name : str,
                 rdf_rec     : RDataFrame,
                 rdf_gen     : Union[RDataFrame,None] = None):
        '''
        sample_name: The name of the MC sample, needed to assign the block number, e.g. `mc_24_w31_34_magup_sim10d-splitsim02_11102202_bd_kstgamma_eq_highptgamma_dpc_ss_tuple`
        rdf_gen: ROOT dataframe with generator level candidates (MCDecayTree), by default None
        rdf_rec: ROOT dataframe with reconstructed candidates (DecayTree)

        Two modes are implemented:

        - Only `rdf_rec` is passed: Then the class only assigns columns to this dataframe.
        - Both dataframes are passed: Then the reco tree is used to add columns to the `rdf_gen` dataframe.
        '''
        self._sample_name = sample_name
        self._rdf_rec     = rdf_rec
        self._rdf_gen     = rdf_gen

        self._l_block = self._get_blocks()
    # ---------------------------
    def _get_blocks(self) -> list[int]:
        return [1, 2]
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
