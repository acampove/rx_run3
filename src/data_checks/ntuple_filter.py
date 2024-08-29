import os
import toml
import math
import utils_noroot          as utnr
import data_checks.utilities as utdc

from log_store               import log_store
from importlib.resources     import files
from data_checks.filter_file import FilterFile

log=log_store.add_logger('data_checks:ntuple_filter')
#----------------------------------------------------------------
class ntuple_filter:
    '''
    Class used to filter ntuples from analysis productions. Filtering means:
    1. Picking a subset of the trees.
    2. Picking a subset of the branches.
    '''
    def __init__(self, dataset=None, cfg_ver=None, index=None, ngroup=None):
        '''
        Parameters
        ---------------------
        dataset (str): Dataset used, e.g. dt_2024_turbo
        cfg_ver (str): Type of configuration, e.g. comp (comparison)
        index   (int): Index of subsample to process, they start at zero up to ngroup - 1
        ngroup  (int): Number of groups into which to split filter
        '''

        self._dataset= dataset
        self._cfg_ver= cfg_ver
        self._index  = index
        self._ngroup = ngroup 

        self._cfg_dat= None

        self._initialized = False
    #---------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._cfg_nam = f'{self._dataset}_{self._cfg_ver}'
        self._cfg_dat = utdc.load_config(self._cfg_nam)
        self._set_paths()

        self._initialized = True 
    #---------------------------------------------
    def _set_paths(self):
        '''
        Loads dictionary with:

        kind_of_file -> [PFNs] 

        correspondence
        '''

        json_path = files('data_checks_data').joinpath(f'{self._dataset}_{self._cfg_ver}.json')
        d_path    = utnr.load_json(json_path)
        d_path    = self._reformat(d_path)
        d_path    = self._get_group(d_path)

        self._d_root_path = d_path
    #---------------------------------------------
    def _reformat(self, d_path):
        '''
        Takes dictionary: 

        sample_kind -> [PFNs]

        Returns dictionary

        PFN -> sample_kind

        Plus remove commas, etc from sample_kind
        '''
        log.debug('Reformating')

        d_path_ref = dict()
        for key, l_path in d_path.items():
            key   = key.replace(',', '_')
            d_tmp = { path : key for path in l_path }
            d_path_ref.update(d_tmp)

        return d_path_ref
    #---------------------------------------------
    def _get_group(self, d_path):
        '''
        Takes a dictionary mapping:

        PFN -> sample_kind

        and the total number of PFNs. Returns same dictionary for ith group out of ngroups
        '''
        log.debug('Getting PFN group')

        nfiles = len(d_path)
        if nfiles < self._ngroup:
            log.error(f'Number of files is smaller than number of groups: {nfiles} < {self._ngroup}')
            raise

        log.info(f'Will split {nfiles} files into {self._ngroup} groups')

        group_size = math.floor(nfiles/self._ngroup)
        index_1    = group_size * (self._index + 0)
        index_2    = group_size * (self._index + 1) if self._index + 1 < self._ngroup else None

        log.info(f'Using range: {index_1}-{index_2}')
        l_pfn      = [pfn for pfn in d_path]
        l_pfn.sort()
        l_pfn      = l_pfn[index_1:index_2]
        d_group    = { pfn : d_path[pfn] for pfn in l_pfn}

        return d_group 
    #---------------------------------------------
    def filter(self):
        '''
        Runs filtering
        '''
        self._initialize()

        for pfn, kind in self._d_root_path.items():
            obj=FilterFile(kind=kind, file_path=pfn, cfg_nam=self._cfg_nam)
            obj.run()
#----------------------------------------------------------------

