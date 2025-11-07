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
    def __init__(self, sample : str) -> None:
        '''
        Parameters
        -------------
        sample : Name of sample, e.g. Bs_JpsiX_ee_eq_JpsiInAcc
        '''
        self._sample = sample 
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
    def patch(self, spec : Specification) -> Specification:
        '''
        Parameters
        -------------
        spec: Specification needed to build ROOT dataframe

        Returns
        -------------
        Patched version, which takes into account block patching
        '''
        return spec
# ------------------------------------
