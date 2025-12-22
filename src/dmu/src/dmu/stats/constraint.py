'''
Module containing constraint classes 
'''

import zfit
import numpy

from functools             import cached_property
from zfit.constraint       import GaussianConstraint as GConstraint
from zfit.constraint       import PoissonConstraint  as PConstraint
from zfit.param            import Parameter as zpar

from pydantic              import BaseModel, ConfigDict
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:constraint')
# ----------------------------------------
class GaussianConstraint(BaseModel):
    '''
    Class representing Gaussian 1D constrain
    '''
    model_config = ConfigDict(arbitrary_types_allowed=True)

    par : zpar 
    mu  : float
    sg  : float
    # ----------------------
    @cached_property
    def observation(self) -> zpar:
        '''
        Parameter symbolizing mean of Gaussian constraint
        '''
        name = self.par.name
        mu   = zfit.Parameter(
            f'{name}_mu', 
            self.mu, 
            self.mu - 5 * self.sg,
            self.mu + 5 * self.sg)

        return mu
    # ----------------------
    @cached_property
    def zfit_cons(self) -> GConstraint:
        '''
        Zfit constraint corresponding to `par`
        '''
        cons = GConstraint(
            params      = self.par, 
            observation = self.observation, 
            uncertainty = self.sg)

        return cons
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''
        name = self.par.name

        return f'{name:<20}{self.mu:<20}{self.sg:<20}{"Gaussian":<20}'
    # ----------------------
    def resample(self) -> None:
        '''
        Sets the value of the constrained parameter to
        '''
        new_val = numpy.random.normal(loc=self.mu, scale=self.sg)

        self.observation.set_value(new_val)
# ----------------------------------------
class PoissonConstraint(BaseModel):
    '''
    Class representing Poisson constrain
    '''
    model_config = ConfigDict(arbitrary_types_allowed=True)

    par : zpar 
    lam : float
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''
        return f'{self.par:<20}{self.lam:<20}{"NA":<20}{"Poisson":<20}'
    # ----------------------
    @cached_property
    def observation(self) -> zpar:
        '''
        Lambda parameter of constraint
        '''
        name = self.par.name

        return zfit.Parameter(f'{name}_lam', self.lam, 0., 10 * self.lam)
    # ----------------------
    @cached_property
    def zfit_cons(self) -> PConstraint:
        '''
        Parameters
        -------------
        par : Zfit parameter

        Returns
        -------------
        Zfit constraint corresponding to `par`
        '''
        return PConstraint(params=self.par, observation=self.observation)
    # ----------------------
    def resample(self) -> None:
        '''
        Updates constraint by drawing mean value from Poisson distribution
        '''
        new_val = numpy.random.poisson(lam = self.lam)

        self.observation.set_value(new_val)
# ----------------------------------------
def print_constraints(constraints : list[GaussianConstraint | PoissonConstraint]) -> None:
    '''
    Parameters
    -------------
    List of constraint objects
    '''
    log.info(f'{"Parameter":<20}{"Value":<20}{"Error":<20}{"Kind":<20}')
    for constraint in constraints:
        log.info(constraint)
# ----------------------------------------
