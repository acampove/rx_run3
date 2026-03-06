'''
Module containing MVAConf class
'''
import re

from typing    import Final
from pydantic  import BaseModel, RootModel, model_validator
from rx_common import MVA

_MVA_REGEX : Final[str] = r'(\d{3})(-\d{3})?_(\d{3})(-\d{3})?'
# ---------------------
class MVAWp(RootModel):
    '''
    Class meant to represent an MVA working point
    '''
    root : float | tuple[float,float]
    # -------------    
    @model_validator(mode = 'after')
    def validate_value(self) -> 'MVAWp':
        if isinstance(self.root, float):
            self._validate_prob(prob = self.root)
            return self

        low, high = self.root # type: ignore[misc]

        if low >= high:
            raise ValueError(f'Invalid working point: {self}')

        self._validate_prob(prob =  low)
        self._validate_prob(prob = high)

        return self
    # -------------    
    def _validate_prob(self, prob : float):
        if 0 <= prob <= 1:
            return
        
        raise ValueError(f'Invalid probability: {prob}')
    # -------------    
    def __str__(self):
        if isinstance(self.root, float):
            return f'{self.root:.3f}'

        low, high  = self.root # type: ignore[misc]

        return f'{low:.3f}-{high:.3f}'
# ---------------------
class MVAConf(BaseModel):
    '''
    Class meant to hold configuration for MVA
    working point
    '''
    cmb : MVAWp
    prc : MVAWp
    # ----------------------
    @classmethod
    def from_values(
        cls,
        cmb : float | tuple[float,float],
        prc : float | tuple[float,float]) -> 'MVAConf':
        '''
        Builds MVAConf from numerical values of working point
        '''
        mva_cmb = MVAWp(root = cmb)
        mva_prc = MVAWp(root = prc)

        return cls(cmb = mva_cmb, prc = mva_prc)
    # ----------------------
    @staticmethod
    def str_to_wp(value : str, kind : MVA) -> tuple[float,float|None]:
        '''
        Parameters
        -------------
        value: String representation of Working point, e.g. 030_020, 030-050_020
        kind : Either prc or cmb, first element is cmb in string

        Returns
        -------------
        Tuple with low bound and optionally high bound
        '''
        mtch = re.match(_MVA_REGEX, value)
        if not mtch:
            raise ValueError(f'Invalid MVA WP string: {value}')

        match kind:
            case MVA.cmb:
                low  = mtch.group(1)
                high = mtch.group(2)
            case MVA.prc:
                low  = mtch.group(3)
                high = mtch.group(4)

        low  = float(low ) / 100.

        if high is not None:
            high = high.lstrip('-')
            high = float(high) / 100.

        return low, high
    # -------------    
    def __str__(self):
        value = f'CMB: {self.cmb}\n'
        value+= f'PRC: {self.prc}'

        return value
# ---------------------
