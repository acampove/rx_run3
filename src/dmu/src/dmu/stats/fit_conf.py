'''
Module holding FitConf class
'''
from pydantic        import BaseModel, ConfigDict
from typing          import Self, Final
from .constraint     import Constraint

_PVALUE : Final[float] = 0.05
#------------------------------
# Strategies
#------------------------------
class Retries(BaseModel):
    '''
    Constrols fit when it is retried multiple times
    '''
    model_config = ConfigDict(frozen=True)

    ntries : int
#------------------------------
class Context(BaseModel):
    '''
    Used if minimization should be run in a separate process
    '''
    model_config = ConfigDict(frozen=True)
#------------------------------
# Minimization itself
#------------------------------
class Minimization(BaseModel):
    '''
    Used to hold parameters needed for zfit's Minuit class 
    '''
    model_config = ConfigDict(frozen=True)

    mode    : int  = 0
    gradient: str  = 'zfit'
#------------------------------
class FitConf(BaseModel):
    '''
    Class meant to define how a fit will be done
    '''
    model_config = ConfigDict(frozen=True)

    pvalue      : float
    minimization: Minimization                     = Minimization()
    nentries    : int                              = -1 
    do_errors   : bool                             = True
    strategy    : Retries | Context         | None = None
    ranges      : list[tuple[float,float] ] | None = None
    constraints : list[Constraint]          | None = None
    # --------------------------
    @classmethod
    def default(cls) -> Self:
        '''
        Returns
        ----------
        Default fit configuration for minimal fit
        '''
        return cls(pvalue = _PVALUE)
#------------------------------
