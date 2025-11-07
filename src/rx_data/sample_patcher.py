'''
Module containing SamplePatcher
'''
from rx_data.specification  import Specification

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
