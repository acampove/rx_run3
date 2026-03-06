'''
Module containing MVAConf class
'''

from pydantic import BaseModel, RootModel, model_validator

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
    # -------------    
    def __str__(self):
        value = f'CMB: {self.cmb}\n'
        value+= f'PRC: {self.prc}'

        return value
# ---------------------
