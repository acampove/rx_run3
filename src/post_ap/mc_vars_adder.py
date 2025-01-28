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
        self._regex       = r'mc_\d{2}_(w\d{2}_\d{2})_.*'

        self._l_block     = self._get_blocks()
        log.debug(f'Using blocks {self._l_block} for sample {self._sample_name}')
    # ---------------------------
    def _get_blocks(self) -> list[int]:
        '''
        Associations taken from:

        https://lhcb-simulation.web.cern.ch/WPP/MCsamples.html#samples
        '''
        log.debug('Picking up blocks')

        mtch = re.match(self._regex, self._sample_name)
        if not mtch:
            raise ValueError(f'Cannot extract block identifier from sample: {self._sample_name}')

        identifier = mtch.group(1)

        if identifier == 'W31_34':
            return [1, 2]

        if identifier == 'W25_27':
            return [4]

        if identifier in ['W35_37']:
            return [5]

        if identifier in ['W37_39']:
            return [6]

        if identifier == 'W40_42':
            return [7, 8]

        raise ValueError(f'Invalid identifier: {identifier}')
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
