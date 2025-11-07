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
        self._spec   = spec
        self._redefinitions : None | dict[str,str] = None
        self._associations  : dict[int,int] = self._get_patching_dictionary()
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
    def get_patched_specification(self) -> Specification:
        '''
        Parameters
        -------------
        spec: Specification needed to build ROOT dataframe

        Returns
        -------------
        Patched version, which takes into account block patching
        '''

        return self._spec
# ------------------------------------
