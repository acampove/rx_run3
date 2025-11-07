'''
Module containing SamplePatcher
'''
from ROOT                   import RDataFrame
from pathlib                import Path
from dmu.generic            import utilities as gut 
from rx_data.specification  import Specification, Sample
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_data:sample_patcher')
# ------------------------------------
class SamplePatcher:
    '''
    Class in charge of modifying specifications to patch for missing
    blocks in simulated samples
    '''
    # ----------------------
    def __init__(self, sample : str, spec : Specification) -> None:
        '''
        Parameters
        -------------
        sample : Name of sample, e.g. Bs_JpsiX_ee_eq_JpsiInAcc
        '''
        self._sample = sample
        self._spec   = spec.model_copy(deep=True)
        self._redefinitions  : None | dict[str,str] = None
        self._associations   : dict[int,int]        = self._get_patching_dictionary()
        self._patching_files : dict[int,list[Path]] = dict()
    # ----------------------
    def _get_patching_dictionary(self) -> dict[int,int]:
        '''
        This method will set redefinitions to an empty dictionary if patching
        not found

        Returns
        -------------
        Dictionary where the keys are the blocks needed 
        and the values are the blocks used to patch the needed blocks

        If no patching is available, dictionary will be empty
        '''
        cfg = gut.load_conf(package='rx_data_data', fpath='emulated_trees/config.yaml')
        if self._sample not in cfg:
            log.info(f'No patching needed for {self._sample}')
            self._redefinitions = dict()
            return dict()

        if 'patching' not in cfg[self._sample]:
            log.info(f'No patching needed for {self._sample}')
            self._redefinitions = dict()
            return dict()

        log.info(f'Using patching for {self._sample}')

        return cfg[self._sample].patching
    # ----------------------
    @property
    def redefinitions(self) -> dict[str,str]:
        '''
        Returns
        --------------
        Dictionary between column name and string with new definition
        '''
        if self._redefinitions is None:
            raise ValueError(f'No redefinitions available for sample {self._sample}')

        return self._redefinitions
    # ----------------------
    def _get_patching_files(self, block : int, main_sample : Sample) -> list[Path]:
        '''
        Parameters
        -------------
        block      : Block that will be used for patching
        main_sample: Object containing list of paths to files in the main sample 

        Returns
        -------------
        List of paths to files containing block
        '''
        if block in self._patching_files:
            return self._patching_files[block]

        paths  = main_sample.files
        [tree] = main_sample.trees

        log.info(f'Finding patching files for block {block}')
        patching_paths : list[Path] = []
        for path in paths:
            rdf       = RDataFrame(tree, str(path))
            arr_block = rdf.AsNumpy(['block'])['block']
            if block not in arr_block:
                continue

            patching_paths.append(path)

        self._patching_files[block] = patching_paths

        return patching_paths
    # ----------------------
    def _patch_sample(self, sample : Sample, patching_files : dict[int,list[Path]]) -> Sample:
        '''
        Parameters
        -------------
        sample: Object holding list of paths to be patched
        patching_files: Dictionary with:
            Key  : Block numbers needed
            Value: List of paths of main sample that should be used for patching. 
                   These are full paths and only the file names are meant to be used
                   to identify the patching path

        Returns
        -------------
        Input object, but with patching paths added
        '''
        for block_needed, file_paths in patching_files.items():
            file_names = [ file_path.name for file_path in file_paths ] 
            sample     = self._patch_sample_for_block(block_needed = block_needed, file_names = file_names, sample = sample)

        return sample
    # ----------------------
    def _patch_sample_for_block(
        self, 
        sample       : Sample,
        block_needed : int, 
        file_names   : list[str]) -> Sample:
        '''
        Parameters
        -------------
        block_needed: Block that needs to be patched
        file_names  : Names (not paths) of files that will be used to patch `block_needed`

        Returns
        -------------
        Patched sample
        '''
        patching_paths : set[Path] = { path for path in sample.files if path.name in file_names }
        nfound    = len(patching_paths)
        nexpected = len(file_names)

        if nfound != nexpected:
            log.error('Files from main category')
            for expected in file_names:
                log.info(expected)

            log.error('Files from friend category')
            for found in patching_paths:
                log.info(found.name)

            raise ValueError(f'Size of patching paths is different from expected number of files for block {block_needed}')

        old_sample_size = sample.size
        sample.files    = sample.files + sorted(list(patching_paths))
        new_sample_size = sample.size

        if self._redefinitions is None:
            self._redefinitions = dict()

        redefinition = f' rdfentry_ + 1 > {old_sample_size} && rdfentry_ < {new_sample_size} ? {block_needed} : block'
        log.debug(f'Redefinition: {redefinition}')

        self._redefinitions['block'] = redefinition

        return sample
    # ----------------------
    def get_patched_specification(self) -> Specification:
        '''
        Parameters
        -------------
        spec: Specification needed to build ROOT dataframe

        Returns
        -------------
        Patched version, which takes into account block patching
        '''
        if not self._associations:
            log.debug('Returning unpatched specification')
            return self._spec

        main_sample : Sample = self._spec.samples['main']

        d_patching_files : dict[int, list[Path]] = dict()
        for needed_block, patching_block in self._associations.items():
            d_patching_files[needed_block] = self._get_patching_files(block=patching_block, main_sample=main_sample)

        patched_friends : dict[str, Sample] = dict()
        for friend_name, friend_sample in self._spec.friends.items():
            log.debug(f'Patching friend: {friend_name}')
            patched_friends[friend_name] = self._patch_sample(sample=friend_sample, patching_files = d_patching_files)

        log.debug('Patching main tree')
        patched_main = self._patch_sample(sample=main_sample, patching_files = d_patching_files)
        data         = {'friends' : patched_friends, 'samples' : {'main' : patched_main }}

        return Specification(**data) 
# ------------------------------------
