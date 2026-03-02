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
    model_config = ConfigDict(frozen=True)

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
    model_config = ConfigDict(frozen=True)

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
class YieldConf(RootModel):
    '''
    Class meant to wrap SimpleYieldConf and CompositeYieldConf
    pydantic models
    '''
    root : SimpleYieldConf | CompositeYieldConf
# ------------------------------
class YieldsConf(RootModel):
    '''
    Class meant to represent dictionary between
    parameter and configuration
    '''
    model_config = ConfigDict(frozen=True)

    root : dict[str, YieldConf]
    # --------------------
    def __iter__(self):  # type: ignore[override]
        return iter(self.root.items())
    # --------------------
    def __contains__(self, item: object) -> bool:
        return item in self.root
    # --------------------
    def __getitem__(self, key: str) -> YieldConf:
        return self.root[key]
    # --------------------
    def __setitem__(self, key: str, val : YieldConf) -> None:
        self.root[key].root = val.root
# ------------------------------
