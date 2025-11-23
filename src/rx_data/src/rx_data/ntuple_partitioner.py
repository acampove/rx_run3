'''
This module contains the NtuplePartitioner class
'''
import os

from pathlib   import Path
from dmu       import LogStore
from rx_common import Project

log=LogStore.add_logger('rx_data:ntuple_partitioner')
# -------------------------------------------
class NtuplePartitioner:
    '''
    This class will:

    - Load paths for ntuples
    - Provide a subset of paths according to user-provided partition
    '''
    # ----------------------
    def __init__(
        self, 
        project : Project,
        kind    : str):
        '''
        Parameters
        -------------
        kind: Type of tree, e.g. main, mva, hop...
        '''
        self._kind    = kind
        self._proj    = project
        self._ana_dir = Path(os.environ['ANADIR'])
    # ----------------------
    def partition(self, index : int, total : int) -> set[Path]:
        '''
        Parameters
        -------------
        index: Index of the partition
        total: Number of partitions

        Returns
        -------------
        Set of paths making up the requested partition
        '''
        dir_path = self._ana_dir / f'Data/{self._proj}/{self._kind}'
        log.debug(f'Looking for latest version in: {dir_path}')

        data_dir = vman.get_last_version(dir_path = dir_path, version_only = False)
        log.debug(f'Lookig for files in: {data_dir}')

        l_path   = list(data_dir.glob('*.root'))
        l_path   = _filter_paths(l_path)
        l_path   = _get_partition(l_path)

        nfiles   = len(l_path)
        if nfiles == 0:
            raise ValueError(f'No file found in: {data_dir}')

        log.info(f'Picking up {nfiles} file(s) from {data_dir}')

        return l_path
# -------------------------------------------
