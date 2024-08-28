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
        Loads groups of paths to ROOT files in EOS
        '''

        json_path = files('data_checks_data').joinpath(f'{self._dataset}.json')
        l_path    = utnr.load_json(json_path)
        l_path    = self._get_group(l_path)

        self._l_root_path = l_path
    #---------------------------------------------
    def _get_group(self, l_path):
        '''
        Takes a list of PFNs and returns the list of PFNs
        associated to `self._index` group out of `ngroup`
        '''
        nfiles = len(l_path)

        if nfiles < self._ngroup:
            log.error(f'Number of files is smaller than number of groups: {nfiles} < {self._ngroup}')
            raise

        log.info(f'Will split {nfiles} files into {self._ngroup} groups')

        group_size = math.floor(nfiles/self._ngroup)
        index_1    = group_size * (self._index + 0)
        index_2    = group_size * (self._index + 1) if self._index + 1 < self._ngroup else None

        log.info(f'Using range: {index_1}-{index_2}')

        l_path_group = l_path[index_1:index_2]

        return l_path_group
    #---------------------------------------------
    def filter(self):
        '''
        Runs filtering
        '''
        self._initialize()

        for root_path in self._l_root_path:
            obj=FilterFile(file_path=root_path, cfg_nam=self._cfg_nam)
            obj.run()
#----------------------------------------------------------------

