'''
Module containing CacheData class
'''
# pylint: disable = too-many-instance-attributes, too-few-public-methods, import-error

import os

from dmu.logging.log_store import LogStore

from rx_selection.ds_getter import ds_getter as dsg

log = LogStore.add_logger('rx_selection:cache_data')
# ----------------------------------------
class CacheData:
    '''
    Class used to apply selection to input datasets and save selected files
    It's mostly an interface to ds_getter
    '''
    # ----------------------------------------
    def __init__(self, cfg : dict, ipart : int, npart : int):
        self._cfg         = cfg
        self._ipart       = ipart
        self._npart       = npart

        self._proc        = cfg['proc']
        self._vers        = cfg['vers']
        self._dset        = cfg['dset']
        self._trig        = cfg['trig']
        self._q2bin       = cfg['q2bin']
        self._preffix     = cfg['preffix']
        self._selection   = cfg['selection']
        self._truth_corr  = cfg['truth_corr']
        self._l_skip_cut  = cfg['skip_cuts']
        self._skip_cmb_bdt= cfg['skip_cmb_bdt']
        self._skip_prc_bdt= cfg['skip_prc_bdt']

        self._chan        : str
        self._tree        : str
        self._cache_dir   : str

        self._setup_vars()
    # ----------------------------------------
    def _setup_vars(self) -> None:
        if   self._trig in ['ETOS', 'GTIS']:
            self._chan = 'ee'
            self._tree = 'KEE'
        elif self._trig in ['MTOS']:
            self._chan = 'mm'
            self._tree = 'KMM'
        else:
            log.error(f'Invalid trigger: {self._trig}')
            raise ValueError

        self._cache_dir = os.environ['CASDIR']
    # ----------------------------------------
    def _cache_path(self) -> tuple[str, bool]:
        '''
        Picks name of directory where samples will go
        Checks if ROOT file is already made
        Returns path and flag, signaling that the file exists or not
        '''
        path_dir = f'{self._cache_dir}/tools/apply_selection/{self._preffix}/{self._proc}/{self._vers}/{self._dset}_{self._trig}'
        os.makedirs(path_dir, exist_ok=True)

        path     = f'{path_dir}/{self._ipart}_{self._npart}.root'
        if os.path.isfile(path):
            log.info(f'Loading cached data: {path}')
            return path, True

        return path, False
    # ----------------------------------------
    def _get_tree_name(self) -> str:
        '''
        Will return KEE or KMM depending on L0 trigger
        '''
        if self._trig not in self._l_skip_cut:
            return self._trig

        if   self._trig == 'MTOS':
            name = 'KMM'
        elif self._trig in ['ETOS', 'GTIS']:
            name = 'KEE'
        else:
            raise ValueError(f'Invalid/Unsupported trigger: {self._trig}')

        log.info(f'Using non-trigger tree name: {name}')

        return name
    # ----------------------------------------
    def save(self) -> None:
        '''
        Will apply selection and save ROOT file
        '''
        ntp_path, is_cached = self._cache_path()
        if is_cached:
            return

        part       = (self._ipart, self._npart)
        d_redefine = { cut : '(1)' for cut in self._l_skip_cut }

        obj = dsg(
                self._q2bin,
                self._trig,
                self._dset,
                self._vers,
                part,
                self._proc,
                self._selection)

        rdf = obj.get_df(
                d_redefine= d_redefine,
                skip_prec = self._skip_prc_bdt,
                skip_cmb  = self._skip_cmb_bdt)

        cfl_path = ntp_path.replace('.root', '.json')
        log.info(f'Saving to: {cfl_path}')
        log.info(f'Saving to: {ntp_path}:{self._trig}')

        rdf.cf.to_json(cfl_path)

        tree_name = self._get_tree_name()

        rdf.Snapshot(tree_name, ntp_path)
# ----------------------------------------
