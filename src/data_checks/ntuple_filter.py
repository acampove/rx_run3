
import os
import apd
import toml
import math

from data_checks.filter_file import FilterFile
from log_store               import log_store
from importlib.resources     import files


log=log_store.add_logger('data_checks:ntuple_filter')
#----------------------------------------------------------------
class ntuple_filter:
    '''
    Class used to filter ntuples from analysis productions. Filtering means:
    1. Picking a subset of the trees.
    2. Picking a subset of the branches.
    '''
    def __init__(self, cfg_nam=None, index=None):
        '''
        Parameters
        ---------------------
        cfg_nam (str): Name of config file used to specify input sample
        index   (int): Index of subsample to process, they start at zero up to ngroups - 1
        '''

        self._cfg_nam= cfg_nam
        self._index  = index

        self._cfg_dat= None

        self._initialized = False
    #---------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._load_config()
        self._set_paths()

        self._initialized = True 
    #---------------------------------------------
    def _load_config(self):
        '''
        Will load TOML file with configuration and set config in self._cfg attribute
        '''
        cfg_path = files('data_checks_data').joinpath(f'config/{self._cfg_nam}.toml')
        if not os.path.isfile(cfg_path):
            log.error(f'Cannot find config: {cfg_path}')
            raise FileNotFoundError

        self._cfg_dat = toml.load(cfg_path)
    #---------------------------------------------
    def _set_paths(self):
        '''
        Uses apd to retrieve list of PFNs corresponding to `self._index` group
        '''
        d_samp= self._cfg_dat['sample']
        d_prod= self._cfg_dat['production']

        obj    = apd.get_analysis_data(**d_prod)
        l_path = obj(**d_samp)
        l_path.sort()
        l_path = self._get_group(l_path)

        self._l_root_path = l_path
    #---------------------------------------------
    def _get_group(self, l_path):
        '''
        Takes a list of PFNs and returns the list of PFNs
        associated to `self._index` group out of `ngroup`
        '''
        ngroup = self._cfg_dat['splitting']['groups']
        nfiles = len(l_path)

        if nfiles < ngroup:
            log.error(f'Number of files is smaller than number of groups: {nfiles} < {ngroup}')
            raise

        log.info(f'Will split {nfiles} files into {ngroup} groups')

        group_size = math.floor(nfiles/ngroup)
        index_1    = group_size * (self._index + 0)
        index_2    = group_size * (self._index + 1) if self._index + 1 < ngroup else None

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














































