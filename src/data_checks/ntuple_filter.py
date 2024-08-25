
import os
import apd
import toml
import math

from log_store           import log_store
from importlib.resources import files


log=log_store.add_logger('data_checks:ntuple_filter')
#----------------------------------------------------------------
class ntuple_filter:
    '''
    Class used to filter ntuples from analysis productions. Filtering means:
    1. Picking a subset of the trees.
    2. Picking a subset of the branches.
    '''
    def __init__(self, cfg=None, index=None):
        '''
        Parameters
        ---------------------
        cfg    (str): Name of config file used to specify input sample
        index  (int): Index of subsample to process, they start at zero up to ngroups - 1
        '''

        self._cfg    = cfg
        self._index  = index

        self._cfg    = None

        self._initialized = False
    #---------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._load_config()
        self._set_paths()
        self._check_index()

        self._initialized = True 
    #---------------------------------------------
    def _load_config(self):
        '''
        Will load TOML file with configuration and set config in self._cfg attribute
        '''
        cfg_path = files('data_checks_data').joinpath(f'config/{self._cfg}.toml')
        if not os.path.isfile(cfg_path):
            log.error(f'Cannot find config: {cfg_path}')
            raise FileNotFoundError

        self._cfg = toml.load(cfg_path)
    #---------------------------------------------
    def _set_paths(self):
        '''
        Uses apd to retrieve list of PFNs corresponding to `self._index` group
        '''
        d_sam = self._cfg['sample']
        d_prod= self._cfg['production']

        obj    = apd.get_analysis_data(**d_prod)
        l_path = obj(**d_samp)
        l_path.sort()
        l_path = self._get_group(l_path)

        return l_path
    #---------------------------------------------
    def _get_group(self, l_path):
        '''
        Takes a list of PFNs and returns the list of PFNs
        associated to `self._index` group out of `ngroup`
        '''
        ngroup = self._cfg['splitting']['groups']
        nfiles = len(l_path)

        if nfiles < ngroup:
            log.error(f'Number of files is smaller than number of groups: {nfiles} < {ngroup}')
            raise

        log.info(f'Will split {nfiles} files into {ngroup} groups')

        group_size = math.floor(nfiles/ngroup)
        index_1    = group_size * self._index
        index_2    = group_size * self._index + 1 if self._index + 1 < ngroup else None

        log.info(f'Using range: {index_1}-{index_2}')

        l_path_group = l_path[index_1:index_2]

        return l_path_group
    #---------------------------------------------
    def _filter_file(self, path):
        log.debug(f'Filtering: {path}')
    #---------------------------------------------
    def filter(self):
        '''
        Runs filtering
        '''
        self._initialize()

        for root_path in self._l_root_path:
            self._filter_file(root_path)
#----------------------------------------------------------------














































