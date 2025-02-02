'''
Module holding RDFGetter class
'''

import os
from ROOT import RDataFrame, TChain
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:rdf_getter')
# ---------------------------------------------------------------
class RDFGetter:
    '''
    Class meant to load data and MC samples and return them as
    ROOT dataframes
    '''
    samples_dir : str
    # ------------------------------------
    def __init__(self, sample : str, trigger : str):
        self._sample  = sample
        self._trigger = trigger
        self._treename= 'DecayTree'
    # ------------------------------------
    def _get_chain(self, kind : str) -> TChain:
        root_wc = f'{RDFGetter.samples_dir}/{self._sample}/{self._trigger}/*_{kind}.root'

        chain   = TChain(self._treename)

        log.debug(f'Built index for chain made with: {root_wc}')

        nfile   = chain.Add(root_wc)
        if nfile <= 0:
            raise ValueError(f'No files found in: {root_wc}')

        log.debug(f'Adding {nfile} files from {root_wc}')

        return chain
    # ------------------------------------
    def _initialize(self) -> None:
        if not hasattr(RDFGetter, 'samples_dir'):
            raise ValueError('samples_dir attribute has not been set')

        samples_dir = RDFGetter.samples_dir
        if not os.path.isdir(samples_dir):
            raise FileNotFoundError(f'Cannot find: {samples_dir}')
    # ------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Will return ROOT dataframe
        '''
        self._initialize()

        chain_main = self._get_chain(kind ='sample')
        chain_mva  = self._get_chain(kind =   'mva')

        chain_main.AddFriend(chain_mva, 'mva')

        rdf = RDataFrame(chain_main)
        rdf.chains = [chain_main, chain_mva]

        return rdf
# ---------------------------------------------------------------
