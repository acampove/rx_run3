'''
Module containing CacheData class
'''
# pylint: disable = too-many-instance-attributes, too-few-public-methods, import-error

import os

from importlib.resources import files

from dmu.logging.log_store  import LogStore
from rx_selection.ds_getter import ds_getter as dsg

log = LogStore.add_logger('rx_selection:cache_data')
# ----------------------------------------
class CacheData:
    '''
    Class used to apply selection to input datasets and save selected files
    It's mostly an interface to ds_getter
    '''
    # ----------------------------------------
    def __init__(self, cfg : dict):
        self._ipart  : int       = cfg['ipart']
        self._npart  : int       = cfg['npart']

        self._ipath  : str       = cfg['ipath' ]
        self._sample : str       = cfg['sample']
        self._l_rem  : list[str] = cfg['remove']
        self._q2bin  : str       = cfg['q2bin' ]
        self._cutver : str       = cfg['cutver']
        self._hlt2   : str       = cfg['hlt2'  ]
    # ----------------------------------------
    def _get_cut_version(self) -> str:
        cutver = self._cutver
        if cutver != '':
            log.warning(f'Overriding cut version with: {cutver}')
            return cutver

        selection_wc = files('rx_selection_data').joinpath('*.yaml')
        selection_wc = str(selection_wc)
        selection_dir= os.path.dirname(selection_wc)
        version      = vman.get_last_version(selection_dir, 'yaml')

        log.debug('Using latest cut version: {version}')

        return version
    # ----------------------------------------
    def _get_selection_name(self) -> str:
        skipped_cuts   = '_'.join(self._l_rem)
        cutver         = self._get_cut_version()
        selection_name = f'NO_{skipped_cuts}_Q2_{self._q2bin}_VR_{cutver}'

        log.debug(f'Using selection name: {selection_name}')

        return selection_name
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
