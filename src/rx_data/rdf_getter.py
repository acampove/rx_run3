'''
Module holding RDFGetter class
'''

import os
import fnmatch

import yaml
from ROOT                   import RDataFrame, TChain
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:rdf_getter')
# ---------------------------------------------------------------
class RDFGetter:
    '''
    Class meant to load data and MC samples and return them as
    ROOT dataframes
    '''
    samples : dict[str,str]
    # ------------------------------------
    def __init__(self, sample : str, trigger : str):
        self._sample  = sample
        self._trigger = trigger
        self._treename= 'DecayTree'

        self._l_chain : list[TChain] = []
    # ------------------------------------
    def _files_from_yaml(self, path : str) -> list[str]:
        with open(path, encoding='utf-8') as ifile:
            d_data = yaml.safe_load(ifile)

        l_path = []
        log.debug('Finding paths')
        for sample in d_data:
            if not fnmatch.fnmatch(sample, self._sample):
                continue

            log.debug(f'    {sample}')

            l_path += d_data[sample][self._trigger]

        nfile   = len(l_path)
        if nfile <= 0:
            raise ValueError(f'No files found in: {path}')

        log.debug(f'Using {nfile} files from {path}')

        return l_path
    # ------------------------------------
    def _get_chain(self, path : str) -> TChain:
        chain   = TChain(self._treename)

        l_file  = self._files_from_yaml(path)
        for file in l_file:
            chain.Add(file)

        self._l_chain.append(chain)

        return chain
    # ------------------------------------
    def _initialize(self) -> None:
        if not hasattr(RDFGetter, 'samples'):
            raise ValueError('samples attribute has not been set')

        if not isinstance(RDFGetter.samples, dict):
            raise ValueError('samples is not a dictionary')

        nsample = len(RDFGetter.samples)
        if nsample == 0:
            raise FileNotFoundError('No samples found')

        bad_path = False
        for name, path in RDFGetter.samples.items():
            if os.path.isfile(path):
                continue

            bad_path = True
            log.error(f'Missing path: {path}:{name}')

        if bad_path:
            raise FileNotFoundError('Paths missing')
    # ------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Will return ROOT dataframe
        '''
        self._initialize()

        chain_main = None
        for kind, path in RDFGetter.samples.items():
            log.debug(f'Building chain for {kind} category')

            chain = self._get_chain(path = path)
            if chain_main is None:
                chain_main = chain
                continue

            chain_main.AddFriend(chain, kind)

        rdf = RDataFrame(chain_main)
        rdf.chains = self._l_chain

        return rdf
# ---------------------------------------------------------------
