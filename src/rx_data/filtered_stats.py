'''
This module contains the FilteredStats class
'''

from importlib.resources import files
from pathlib             import Path

import pandas as pnd
from dmu.generic import utilities as gut

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:filtered_stats')
# -------------------------------
class FilteredStats:
    '''
    This class should:

    - Access the JSON files stored in this project with the list of LFNs
    - Use them to build a dataframe with the following columns:
        - Block: e.g. w40_42
        - EventType 
        - Version: version of ntuples in AP 
    '''
    # ----------------------
    def __init__(
        self, 
        analysis : str,
        min_vers : int) -> None:
        '''
        Parameters
        -------------
        analysis: E.g. rx, nopid
        min_vers: Minimum version, will ignore anything before this
        '''
        self._analysis = analysis
        self._min_vers = min_vers
    # ----------------------
    def _lines_from_files(self, l_path : list[Path]) -> int:
        '''
        Parameters
        -------------
        l_path: List of paths to JSON files, each with a list of LFNs

        Returns
        -------------
        Number of LFNs
        '''
        nlfn = 0
        for path in l_path:
            with open(path, 'r') as f:
                nlfn += sum(1 for _ in f)

        return nlfn
    # ----------------------
    @staticmethod
    def _version_from_path(element : Path) -> int:
        '''
        Parameters
        -------------
        element: Element in container to be sorted 

        Returns
        -------------
        Integer representing the order in which the sorting should be done
        '''
        name = element.name

        if not name.startswith('v'):
            raise ValueError(f'Name in path not a version: {name}')

        version = name.replace('v', '')
        if not version.isdigit():
            raise ValueError(f'Version is not digit: {version}')

        return int(version)
    # ----------------------
    def _skip_path(self, path : Path) -> bool:
        '''
        Parameters
        -------------
        path: Path to versioned directory

        Returns
        -------------
        True if it will be skipped
        '''
        numeric_version = self._version_from_path(element=path)

        if numeric_version < self._min_vers:
            return True

        return False
    # ----------------------
    def _get_paths(self) -> list[Path]:
        '''
        Returns
        -------------
        List of paths to JSON files with LFNs
        '''
        base = files('rx_data_lfns').joinpath(self._analysis)
        base = Path(str(base))

        l_dir= [ vers_dir for vers_dir in base.glob('v*') if vers_dir.is_dir() ]
        ndir = len(l_dir)
        if ndir == 0:
            raise ValueError(f'No LFN directories found in: {base}')
        l_dir= sorted(l_dir, key=self._version_from_path)

        l_file = []
        log.info(80 * '-')
        log.info(f'{"Files":<10}{"LFNs":<20}{"Path"}')
        log.info(80 * '-')
        for dir_path in l_dir:
            if self._skip_path(path=dir_path):
                log.debug(f'Skipping: {dir_path}')
                continue

            jfiles  = list(dir_path.glob('*.json'))
            nfiles  = len(jfiles)
            nlines  = self._lines_from_files(l_path=jfiles)
            if nfiles == 0:
                raise ValueError(f'No files found in {dir_path}')
            else:
                log.info(f'{nfiles:<10}{nlines:<20,}{dir_path}')

            l_file += jfiles
        log.info(80 * '-')

        ntot_files = len(l_file)
        log.info(f'Found {ntot_files} LFNs')

        return l_file
    # ----------------------
    def _lfns_from_paths(self, l_path : list[Path]) -> list[str]:
        '''
        Parameters
        -------------
        l_path: List of paths to JSON files storing LFNs

        Returns
        -------------
        List of LFNs
        '''
        l_lfn : list[str] = []
        for path in l_path:
            value  = gut.load_json(path=path)
            # TODO: improve this check
            if not isinstance(value, list):
                raise TypeError(f'Could not load list of strings from: {path}')

            l_lfn += value 

        nlfn = len(l_lfn)
        if nlfn == 0:
            raise ValueError('No LFNs found')

        log.debug(f'Found {nlfn} LFNs')

        return l_lfn
    # ----------------------
    def get_df(self) -> pnd.DataFrame:
        '''
        Returns
        -------------
        Pandas dataframe with requested information
        '''
        l_path = self._get_paths()
        l_lfn  = self._lfns_from_paths(l_path = l_path)

        return
        df     = self._df_from_lfns(l_lfn = l_lfn)

        return df
# -------------------------------
