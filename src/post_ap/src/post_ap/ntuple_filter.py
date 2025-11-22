'''
Module with definition of ntuple_filter class
'''
import os
import math
import json
from dmu.logging.log_store   import LogStore

import post_ap.utilities   as utdc
from   post_ap.filter_file   import FilterFile

log = LogStore.add_logger('post_ap:ntuple_filter')
# ----------------------------------------------------------------
class NtupleFilter:
    '''
    Class used to filter ntuples from analysis productions. Filtering means:
    1. Picking a subset of the trees.
    2. Picking a subset of the branches.
    '''
    # ---------------------------------------------
    def __init__(self, index : int, ngroup : int):
        '''
        Parameters
        ---------------------
        index      : Index of subsample to process, they start at zero up to ngroup - 1
        ngroup     : Number of groups into which to split filter
        '''

        self._index   = index
        self._ngroup  = ngroup

        self._cfg_dat     : dict
        self._d_root_path : dict[str,str]

        self._initialized = False
    # ---------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._cfg_dat = utdc.load_config()
        self._set_paths()

        self._initialized = True
    # ---------------------------------------------
    def _invert_dictionary(self, d_pfn : dict[str,list[str]]) -> dict[str,str]:
        d_inv_pfn = {}
        for kind, l_pfn in d_pfn.items():
            d_tmp = { pfn : kind for pfn in l_pfn }

            d_inv_pfn.update(d_tmp)

        return d_inv_pfn
    # ---------------------------------------------
    def _set_paths(self):
        '''
        Loads dictionary with:

        kind_of_file -> [PFNs]

        correspondence
        '''
        pfn_path = os.environ['PFN_PATH']
        with open(pfn_path, encoding='utf-8') as ifile:
            d_pfn = json.load(ifile)

        d_pfn  = self._invert_dictionary(d_pfn)
        d_path = self._get_group(d_pfn)

        self._d_root_path = d_path
    # ---------------------------------------------
    def _load_json(self, json_path : str) -> dict[str, list[str]]:
        if not os.path.isfile(json_path):
            raise FileNotFoundError(f'File not found: {json_path}')

        with open(json_path, encoding='utf-8') as ifile:
            return json.load(ifile)
    # ---------------------------------------------
    def _reformat(self, d_path : dict[str, list[str]]) -> dict[str,str]:
        '''
        Takes dictionary:

        sample_kind -> [PFNs]

        Returns dictionary

        PFN -> sample_kind

        Plus remove commas, etc from sample_kind
        '''
        log.debug('Reformating')

        d_path_ref = {}
        for key, l_path in d_path.items():
            key   = key.replace(',', '_')
            d_tmp = { path : key for path in l_path }
            d_path_ref.update(d_tmp)

        return d_path_ref
    # ---------------------------------------------
    def _get_group(self, d_path : dict[str,str]) -> dict[str,str]:
        '''
        Takes a dictionary mapping:

        PFN -> sample_kind

        Returns same dictionary for ith group out of ngroups
        '''
        log.debug('Getting PFN group')

        nfiles = len(d_path)
        if nfiles < self._ngroup:
            raise ValueError(f'Number of files is smaller than number of groups: {nfiles} < {self._ngroup}')

        log.info(f'Will split {nfiles} files into {self._ngroup} groups')

        group_size = math.floor(nfiles / self._ngroup)
        index_1    = group_size * (self._index + 0)
        index_2    = group_size * (self._index + 1) if self._index + 1 < self._ngroup else None

        log.info(f'Using range: {index_1}-{index_2}')
        l_pfn      = list(d_path)
        l_pfn.sort()
        l_pfn      = l_pfn[index_1:index_2]
        d_group    = { pfn : d_path[pfn] for pfn in l_pfn}

        return d_group
    # ---------------------------------------------
    def filter(self):
        '''
        Runs filtering
        '''
        self._initialize()

        for pfn, kind in self._d_root_path.items():
            obj = FilterFile(sample_name=kind, file_path=pfn)
            obj.run()
# ----------------------------------------------------------------
