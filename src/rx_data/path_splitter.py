'''
Module containing PathSplitter class
'''
from pathlib import Path
from typing  import TypeAlias

import ap_utilities.decays.utilities as aput
from dmu.logging.log_store  import LogStore
from rx_data                import utilities as ut

Samples       : TypeAlias = dict[tuple[str, str], list[Path]]
NestedSamples : TypeAlias = dict[str, dict[str, list[Path]]]

log   = LogStore.add_logger('rx_data:path_splitter')
# ------------------------------------------
class PathSplitter:
    '''
    Class meant to split lists of LFNs/paths/PFNs of ROOT files into
    Samples and HLT2 trigger categories
    '''
    # ------------------------------------------
    def __init__(
        self, 
        paths         : list[Path], 
        max_files     : int = -1, 
        sample_naming : str = 'new'):
        '''
        paths: List of LFNs/PFNs/Local paths
        max_files: If doing tests, the output lists will be limited to this number, default not truncate
        sample_naming : Either `new` (for Run3) or `old` (for Run1/2 compatibility)
        '''
        self._l_path       = paths
        self._max_files    = max_files
        self._sample_naming= sample_naming
    # ------------------------------------------
    def _truncate_paths(self, samples : Samples) -> Samples: 
        '''
        Will limit the number of paths in the values if Data.Max is larger than zero
        '''
        if self._max_files < 0:
            return samples

        log.warning(f'Truncating to {self._max_files} paths')

        d_path_trunc = { key : val[:self._max_files] for key, val in samples.items() }

        return d_path_trunc
    # ------------------------------------------
    def _rename_sample(self, samples : Samples) -> Samples:
        '''
        Parameters
        ---------------
        samples : Dictionary mapping sample, trigger tuple to list of ROOT file paths

        Returns
        ---------------
        Same as input, with sample names changed to go from lower case (extracted from paths) to 
        mixed case (needed by downstream code)
        '''
        log.debug('Renaming samples from lower-case only')
        d_renamed = {}

        for (sample, line_name), l_fpath in samples.items():
            try:
                sample = aput.name_from_lower_case(sample)
            except ValueError as exc:
                log.warning(f'Cannot find sample name for lowercase name: {sample}')
                log.warning(exc)
                log.warning('')
                continue

            log.debug(f'Using {self._sample_naming} sample_naming for samples')
            if self._sample_naming != 'old' or sample.startswith('DATA_'):
                d_renamed[(sample, line_name)] = l_fpath
                continue

            try:
                sample = aput.old_from_new_nick(nickname=sample)
            except ValueError as exc:
                log.warning(exc)
                continue

            d_renamed[(sample, line_name)] = l_fpath

        return d_renamed
    # ------------------------------------------
    def _nest_samples(self, samples : Samples ) -> NestedSamples:
        '''
        Parameters
        ----------------
        samples: Dictionary mapping sample name and trigger tuple to list of ROOT file paths

        (sample, trigger) : path_1

        Returns
        ----------------
        Nested structure like:

        sample:
            trigger:
                - path 1
                - path 2
                - path 3
        '''
        d_struc : NestedSamples = {}
        for (sample, line), l_path in samples.items():
            if sample not in d_struc:
                d_struc[sample] = {}

            d_struc[sample][line] = l_path

        return d_struc
    # ------------------------------------------
    def split(self, nested : bool = False) -> Samples | NestedSamples:
        '''
        Takes list of paths to ROOT files and splits them in an easier to read structure

        Parameters
        ------------------------
        nested: If False, splits them into categories and returns a dictionary:

        category : [path_1, path_2, ...]

        If True, it will use a nested structure like:

        sample:
            trigger:
                - path 1
                - path 2
                - path 3
        '''
        npath = len(self._l_path)
        log.info(f'Splitting {npath} paths into categories')

        samples : Samples = {}
        for path in self._l_path:
            info = ut.info_from_path(path=path)
            if info not in samples:
                samples[info] = []

            samples[info].append(path)

        samples = self._truncate_paths(samples = samples)
        samples = self._rename_sample(samples = samples)

        log.debug('Found samples:')
        samples = dict(sorted(samples.items()))
        for sample, line in sorted(samples):
            log.debug(f'{sample:<50}{line:<30}')

        if not nested:
            return samples

        return self._nest_samples(samples = samples)
# ------------------------------------------
