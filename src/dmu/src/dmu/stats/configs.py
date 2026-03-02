'''
Module with classes meant to represent configurations needed to build yields
'''

from typing   import Literal, Self
from pydantic import BaseModel, model_validator
from pydantic import RootModel, ConfigDict

# ------------------------------
class SimpleYieldConf(BaseModel):            # Tested
    '''
    Class representing configuration for yields
    '''
    val    : float
    min    : float
    max    : float
    scl    : list[str] | None = None
    prefix : str       | None = None
    # ----------------------------------
    @model_validator(mode = 'after')
    def check_bounds(self) -> Self:
        if not (self.min < self.val < self.max):
            raise ValueError(f'Invalid bounds: {self}')

        return self
# ------------------------------
class CompositeYieldConf(BaseModel):
    '''
    Class meant to represent composite yield
    '''
    kind : Literal['mul', 'dif']
    pars : list[str]
    # ----------------------
    @model_validator(mode = 'after')
    def check_size(self) -> Self:
        if len(self.pars) < 2:
            raise ValueError(f'At least two parameters are needed, found: {self.pars}')

        if self.kind == 'dif' and len(self.pars) != 2:
            raise ValueError(f'Expected two parameters found: {self.pars}')

        return self
# ------------------------------
class YieldConf(BaseModel):
    '''
    Class meant to wrap SimpleYieldConf and CompositeYieldConf
    pydantic models
    '''
    root : SimpleYieldConf | CompositeYieldConf

    @model_validator(mode='before')
    @classmethod
    def _pack_data(cls, data):
        return {'root' :  data}

    def __getattr__(self, item):
        return getattr(self.root, item)
