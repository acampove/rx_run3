'''
Module holding MCVarsAdder class
'''
import re
import random
from typing    import Union
from functools import cached_property

import numpy
import dmu.rdataframe.utilities as ut
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
        self._sample_name     = sample_name
        self._rdf_rec         = rdf_rec
        self._rdf_gen         = rdf_gen
        self._regex           = r'mc_\d{2}_(w\d{2}_\d{2})_.*'
        self._branch_id       = 'branch_id'
        self._block_name      = 'block'
        self._unmatched_trees = False
        self._l_block         = self._get_blocks()

        log.debug(f'Using blocks {self._l_block} for sample {self._sample_name}')

        self._set_branch_id()
        # This number is needed to create new numba functions
        # It has to be as random as possible, no need to make it reproducible
        self._randomid = random.getrandbits(128)

        log.debug(f'Random ID: {self._randomid}')

        # Random seed needs to be fixed to make the analysis reproducible
        self._rng          = numpy.random.default_rng(seed=10)
        self._random_state = random.getstate()
        random.seed(10)
    # ---------------------------
    def __del__(self):
        random.setstate(self._random_state)
    # ---------------------------
    def _set_branch_id(self) -> None:
        '''
        This function will:

        - Check true PT branches in reco tree
        - PT branches in gen tree
        - At least one matching branch
        - Rename it as self._branch_id in each dataframe

        and it should be removed when the EVENTNUMBER branch becomes available in the MCDT trees
        '''
        # This branch ID is needed to match stuff in gen and rec
        # No gen => no matching => no branch ID
        if self._rdf_gen is None:
            log.debug('No branch ID needed')
            return

        l_gen_name = [ name.c_str() for name in self._rdf_gen.GetColumnNames() ]
        l_rec_name = [ name.c_str() for name in self._rdf_rec.GetColumnNames() ]

        l_gen_name = [ name for name in l_gen_name if name.endswith('_PT'    ) ]
        l_rec_name = [ name for name in l_rec_name if name.endswith('_TRUEPT') ]
        l_rec_name_strip = [ name.replace('TRUE', '') for name in l_rec_name ]

        l_common   = [ name for name in l_gen_name if name in l_rec_name_strip ]
        if len(l_common) == 0:
            log.info('Gen branches:')
            log.info(l_gen_name)
            log.info('')
            log.info('Rec branches:')
            log.info(l_rec_name_strip)
            log.warning('Cannot find common PT branches between MCDT and DecayTree')
            self._unmatched_trees = True
            return

        common_gen = l_common[0]
        common_rec = common_gen.replace('_PT', '_TRUEPT')
        log.debug(f'Using common branches: {common_gen}/{common_rec}')

        self._rdf_rec = self._rdf_rec.Define(self._branch_id, common_rec)
        self._rdf_gen = self._rdf_gen.Define(self._branch_id, common_gen)
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

        if identifier == 'w31_34':
            return [1, 2]

        if identifier == 'w25_27':
            return [4]

        if identifier in ['w35_37']:
            return [5]

        if identifier in ['w37_39']:
            return [6]

        if identifier == 'w40_42':
            return [7, 8]

        raise ValueError(f'Invalid identifier: {identifier}')
    # ---------------------------
    def _add_to_rec(self) -> RDataFrame:
        nentries  = self._rdf_rec.Count().GetValue()
        log.debug(f'Adding block column for {nentries} entries')
        arr_block = self._rng.choice(self._l_block, size=nentries)
        rdf       = ut.add_column_with_numba(self._rdf_rec, arr_block, self._block_name, identifier=f'rec_block_{self._randomid}')

        return rdf
    # ---------------------------
    @cached_property
    def _get_rec_identifiers(self) -> list[str]:
        log.debug('Getting identifiers for rec tree')
        l_id = self._get_identifiers(self._rdf_rec)

        return l_id
    # ---------------------------
    @cached_property
    def _get_gen_identifiers(self) -> list[str]:
        log.debug('Getting identifiers for gen tree')
        l_id = self._get_identifiers(self._rdf_gen)

        return l_id
    # ---------------------------
    def _get_identifiers(self, rdf) -> list[str]:
        arr_id_value = rdf.AsNumpy([self._branch_id])[self._branch_id]
        arr_id_scale = arr_id_value * 1000_000
        arr_id_str   = arr_id_scale.astype(int).astype(str)

        return arr_id_str.tolist()
    # ---------------------------
    def _get_mapping(self, name : str) -> dict[str, int]:
        log.debug(f'Get mapping for {name}')

        l_identifier= self._get_rec_identifiers
        arr_target  = self._rdf_rec.AsNumpy([name])[name]
        l_target    = arr_target.tolist()

        nid = len(l_identifier)
        ntg = len(l_target)

        if nid != ntg:
            raise ValueError(f'For target/identifier {name}/{self._branch_id} values differ: {nid}/{ntg}')

        d_map = dict(zip(l_identifier, l_target))

        return d_map
    # ---------------------------
    def _pick_target(self, gen_id : str, mapping : dict[str,int], name : str) -> int:
        if gen_id in mapping:
            return mapping[gen_id]

        if name == self._block_name:
            return random.choice(self._l_block)

        # Making this negative ensures we won't accidentally collide with in-mapping value
        if name == 'EVENTNUMBER':
            return random.randint(-1000_000, 0)

        raise ValueError(f'Cannot pick out of mapping random number for: {name}')
    # ---------------------------
    def _add_to_gen(self) -> RDataFrame:
        rdf     = self._rdf_gen

        d_id_bk = self._get_mapping(name= self._block_name)
        d_id_ev = self._get_mapping(name=    'EVENTNUMBER')
        l_gen_id= self._get_gen_identifiers

        l_ev    = [ self._pick_target(gen_id=gen_id, mapping = d_id_ev, name =    'EVENTNUMBER') for gen_id in l_gen_id ]
        l_bk    = [ self._pick_target(gen_id=gen_id, mapping = d_id_bk, name = self._block_name) for gen_id in l_gen_id ]

        ngen    = len(l_gen_id)
        log.debug(f'Adding columns for {ngen} entries')
        rdf     = ut.add_column_with_numba(rdf, numpy.array(l_bk), self._block_name, identifier=f'gen_block_{self._randomid}')
        rdf     = ut.add_column_with_numba(rdf, numpy.array(l_ev),    'EVENTNUMBER', identifier=f'gen_evtnm_{self._randomid}')

        return rdf
    # ---------------------------
    def _add_to_gen_no_match(self) -> RDataFrame:
        '''
        This function will add branches to MCDT with values that are not matched
        to the ones in the DecayTree. This will only happen if no matching branch
        (e.g. EVENTNUMBER, B_PT) was found.
        '''
        rdf       = self._rdf_gen
        ngen      = rdf.Count().GetValue()

        arr_block = self._rng.choice(self._l_block, size=ngen)
        arr_evtnm = self._rng.integers(-1000_000, 0, size=ngen)

        log.debug(f'Adding columns for {ngen} entries')
        rdf     = ut.add_column_with_numba(rdf, arr_block, self._block_name, identifier=f'gen_block_{self._randomid}')
        rdf     = ut.add_column_with_numba(rdf, arr_evtnm,    'EVENTNUMBER', identifier=f'gen_evtnm_{self._randomid}')

        return rdf
    # ---------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe after adding column
        '''

        if self._rdf_gen is None:
            log.info('Adding MC variables to DecayTree')
            rdf = self._add_to_rec()

            return rdf

        if not self._unmatched_trees:
            log.info('Adding MC variables to MCDecayTree')
            rdf = self._add_to_gen()
        else:
            log.warning('Adding MC variables to MCDecayTree without matching')
            rdf = self._add_to_gen_no_match()

        return rdf
# -----------------------------
