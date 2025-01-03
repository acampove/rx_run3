'''
Module containing CacheData class
'''
# pylint: disable = too-many-instance-attributes, too-few-public-methods, import-error

import os

from importlib.resources import files

from dmu.logging.log_store  import LogStore
from rx_selection.ds_getter import ds_getter          as dsg
from rx_selection           import version_management as vman

log = LogStore.add_logger('rx_selection:cache_data')
# ----------------------------------------
class CacheData:
    '''
    Class used to apply selection to input datasets and save selected files
    It's mostly an interface to ds_getter
    '''
    # ----------------------------------------
    def __init__(self, cfg : dict):
        self._cfg    : dict      = cfg

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
        selection_name = self._get_selection_name()
        opath          = self._ipath.replace('post_ap', selection_name)

        path_dir = f'{opath}/{self._sample}/{self._hlt2}'
        os.makedirs(path_dir, exist_ok=True)

        path     = f'{path_dir}/{self._ipart:03}_{self._npart:03}.root'
        if os.path.isfile(path):
            log.info(f'Loading cached data: {path}')
            return path, True

        return path, False
    # ----------------------------------------
    def _get_dsg_cfg(self) -> dict:
        dsg_cfg             = dict(self._cfg)
        dsg_cfg['redefine'] = { cut : '(1)' for cut in self._l_rem }

        del dsg_cfg['remove']

        return dsg_cfg
    # ----------------------------------------
    def save(self) -> None:
        '''
        Will apply selection and save ROOT file
        '''
        ntp_path, is_cached = self._cache_path()
        if is_cached:
            return

        log.info(f'Path not cached, will create: {ntp_path}')

        dsg_cfg = self._get_dsg_cfg()
        obj     = dsg(cfg=dsg_cfg)
        rdf     = obj.get_rdf()

        cfl_path = ntp_path.replace('.root', '.json')
        log.info(f'Saving to: {cfl_path}')
        log.info(f'Saving to: {ntp_path}')

        rdf.cf.to_json(cfl_path)

        rdf.Snapshot('DecayTree', ntp_path)
# ----------------------------------------
