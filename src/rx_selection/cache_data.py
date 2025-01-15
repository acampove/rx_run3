'''
Module containing CacheData class
'''
# pylint: disable = too-many-instance-attributes, too-few-public-methods, import-error

import os
import glob

from importlib.resources import files

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_selection.ds_getter import DsGetter
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
        self._cutver : str       = '' if 'cutver' not in cfg else cfg['cutver']
        self._hlt2   : str       = cfg['hlt2'  ]
    # ----------------------------------------
    def _get_cut_version(self) -> str:
        cutver = self._cutver
        if cutver != '':
            log.warning(f'Overriding cut version with: {cutver}')
            return cutver

        selection_wc = files('rx_selection_data').joinpath('selection/*.yaml')
        selection_wc = str(selection_wc)
        selection_dir= os.path.dirname(selection_wc)
        version      = vman.get_last_version(selection_dir, 'yaml')

        log.debug(f'Using latest cut version: {version}')

        self._cfg['cutver'] = version

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
    def _save_lumifile(self, rdf : RDataFrame, out_dir : str) -> None:
        if self._ipart != 0:
            return None

        file_path = rdf.filepath[0]
        dir_path  = os.path.dirname(file_path)
        path_wc   = f'{dir_path}/*.root'
        l_path    = glob.glob(path_wc)

        if 'max_files' in self._cfg:
            nmax   = self._cfg['max_files']
            log.warning(f'Limitting lumiTrees to {nmax}')
            l_path = l_path[:nmax]

        npath     = len(l_path)
        log.info(f'Making lumi file from {npath} files')

        tree_name = 'lumiTree'
        file_path = f'{out_dir}/lumi.root'

        if os.path.isfile(file_path):
            log.info(f'Lumi file already found, skipping: {file_path}')
            return None

        log.info(f'Saving lumi file to: {file_path}:{tree_name}')
        rdf       = RDataFrame(tree_name, l_path)
        rdf.Snapshot(tree_name, file_path)

        return None
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
        obj     = DsGetter(cfg=dsg_cfg)
        rdf     = obj.get_rdf()

        cfl_path = ntp_path.replace('.root', '.json')
        log.info(f'Saving to: {cfl_path}')
        log.info(f'Saving to: {ntp_path}')

        rdf.cf.to_json(cfl_path)

        if self._sample.startswith('DATA_'):
            out_dir = os.path.dirname(ntp_path)
            self._save_lumifile(rdf, out_dir)

        rdf.Snapshot('DecayTree', ntp_path)
# ----------------------------------------
