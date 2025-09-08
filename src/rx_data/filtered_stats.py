'''
This module contains the FilteredStats class
'''
import os
import re

from tqdm                import tqdm
from importlib.resources import files
from pathlib             import Path

import pandas as pnd
from dmu.generic        import utilities as gut
from rx_data.ganga_info import GangaInfo

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
        min_vers : int,
        max_lfns : int|None=None) -> None:
        '''
        Parameters
        -------------
        analysis : E.g. rx, nopid
        min_vers : Minimum version, will ignore anything before this
        max_lfns : Maximum number of LFNs per JSON file
        '''
        self._analysis = analysis
        self._min_vers = min_vers
        self._d_lfn    : dict[str,int] = {} 
        self._columns  : list[str]     = ['EventType', 'Sample', 'Trigger', 'Version']

        self._evt_rgx  : str           = r'_(\d{8})_'
        self._trg_rgx  : str           = r'_(Hlt2RD_.*_MVA)_'
        self._sam_rgx  : str           = r'(^mc_.*)_Hlt2RD_.*'
        self._inf = GangaInfo()

        if max_lfns is None:
            return

        if not isinstance(max_lfns, int):
            raise TypeError(f'Invalid value of max_lfns: {max_lfns}')

        self._max_lfns = max_lfns
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
    def _get_paths(self) -> dict[Path, int]:
        '''
        Returns
        -------------
        Dictionary with:
        Key   : Path to JSON file with LFNs
        value : Version of LFNs, e.g. 10
        '''
        base = files('rx_data_lfns').joinpath(self._analysis)
        base = Path(str(base))

        l_dir= [ vers_dir for vers_dir in base.glob('v*') if vers_dir.is_dir() ]
        ndir = len(l_dir)
        if ndir == 0:
            raise ValueError(f'No LFN directories found in: {base}')
        l_dir= sorted(l_dir, key=self._version_from_path)

        d_file : dict[Path, int] = {} 
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

            d_tmp : dict[Path, int] = { path : self._version_from_path(dir_path) for path in jfiles }
            d_file.update(d_tmp)

        log.info(80 * '-')

        ntot_files = len(d_file)
        log.info(f'Found {ntot_files} LFNs')

        return d_file
    # ----------------------
    def _lfns_from_paths(self, d_path : dict[Path, int]) -> dict[str,int]:
        '''
        Parameters
        -------------
        d_path: Dictionary with:
            Key  : Path to JSON file with LFNs
            Value: Version that JSON file belongs to, e.g. 10

        Returns
        -------------
        Dictionary with:
            Key  : LFN
            Value: Version, e.g. 10
        '''
        log.debug('Getting LFNs from paths')

        d_lfn : dict[str,int] = {} 
        for path, version in d_path.items():
            lfns = gut.load_json(path=path)
            # TODO: improve this check
            if not isinstance(lfns, list):
                raise TypeError(f'Could not load list of strings from: {path}')

            if self._max_lfns is not None:
                lfns = lfns[:self._max_lfns]

            d_tmp = {os.path.basename(lfn) : version for lfn in lfns}
            d_lfn.update(d_tmp)

        nlfn = len(d_lfn)
        if nlfn == 0:
            raise ValueError('No LFNs found')

        log.debug(f'Found {nlfn} LFNs')

        return d_lfn
    # ----------------------
    def _add_information(self, row : pnd.Series) -> pnd.Series:
        '''
        Parameters
        -------------
        row: Pandas series representing row of empty dataframe 

        Returns
        -------------
        Series representing updated row
        '''

        row['EventType'] = self._event_type_from_row(row=row) 
        row['Trigger'  ] = self._info_from_row(path=row.path, kind='trigger', regex=self._trg_rgx)
        row['Version'  ] = self._d_lfn[row.path]
        row['Sample'   ] = self._sample_from_row(row=row)
        row['block'    ] = self._inf.block_from_fname(fname=row.path)

        return row 
    # ----------------------
    def _sample_from_row(self, row : pnd.Series) -> str:
        '''
        Parameters
        -------------
        row: Pandas series with path

        Returns
        -------------
        Name of sample
        '''
        path = row.path
        if path.startswith('data'):
            return 'data'

        return self._info_from_row(path=row.path, kind='sample', regex=self._sam_rgx)
    # ----------------------
    def _event_type_from_row(self, row : pnd.Series) -> str:
        '''
        Parameters
        -------------
        row: Row of dataframe with information

        Returns
        -------------
        Event type
        '''
        path = row.path
        if path.startswith('data'):
            return 'data'

        return self._info_from_row(path=path, kind='event type', regex=self._evt_rgx)
    # ----------------------
    def _info_from_row(
        self, 
        path  : str,
        kind  : str, 
        regex : str) -> str:
        '''
        Parameters
        -------------
        path: Name of sample, e.g. mc_magdown_11264001_bd_dmnpipl_eq_dpc_Hlt2RD_BuToKpEE_MVA_5e3bdf6390
        kind: E.g. 'event type'
        regex: E.g. self._evt_rgx

        Returns
        -------------
        Information requested
        '''

        val  = re.search(regex, path)
        if val is None:
            raise ValueError(f'Cannot extract {kind} from: {path}')

        l_info = val.groups()
        if len(l_info) != 1:
            raise ValueError(f'Cannot extract event type from: {path}')

        return l_info[0]
    # ----------------------
    def get_df(self) -> pnd.DataFrame:
        '''
        Returns
        -------------
        Pandas dataframe with requested information
        '''
        d_path       = self._get_paths()
        self._d_lfn  = self._lfns_from_paths(d_path = d_path)
        indexes      = range(len(self._d_lfn))

        tqdm.pandas()
        log.info('Filling dataframe')
        df         = pnd.DataFrame(columns=self._columns, index=indexes)
        df['path'] = list(self._d_lfn)
        df         = df.progress_apply(self._add_information, axis=1) # type: ignore
        df         = df.drop(columns=['path'])

        return df
# -------------------------------
