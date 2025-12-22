'''
Module containing constraint classes 
'''

import zfit
import numpy

from typing                import Protocol
from functools             import cached_property
from zfit.constraint       import GaussianConstraint as GConstraint
from zfit.constraint       import PoissonConstraint  as PConstraint
from zfit.param            import Parameter as zpar

from pydantic              import BaseModel
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:constraint')
# ----------------------------------------
class ParsHolder(Protocol):
    '''
    Class meant to symbolize generic holder of parameters
    '''
    def get_params(self, *args, **kwargs)-> set[zpar]:
        ...
# ----------------------------------------
class ConstraintND(BaseModel):
    '''
    Class meant to symbolize NDimensional Gaussian constraint
    '''
# ----------------------------------------
class Constraint1D(BaseModel):
    '''
    Class representing Gaussian 1D constrain
    '''
    kind: str
    name: str
    mu  : float
    sg  : float
    # ----------------------
    @cached_property
    def observation(self) -> zpar:
        '''
        Parameter symbolizing mean of Gaussian constraint
        '''
        match self.kind:
            case 'GaussianConstraint':
                sg = self.sg
            case 'PoissonConstraint':
                sg = self.mu
            case _:
                raise ValueError(f'Invalid constraint: {self.kind}')

        mu   = zfit.Parameter(
            f'{self.name}_mu', 
            self.mu, 
            self.mu - 5 * sg,
            self.mu + 5 * sg)

        return mu
    # ----------------------
    def _obs_from_holder(self, holder : ParsHolder) -> zpar:
        '''
        Parameters
        ------------------
        holder: Object holding parameters

        Returns
        ------------------
        Observable of constraint, i.e. parameter to constrain
        '''
        s_par = holder.get_params()
        for par in s_par:
            if par.name == self.name:
                return par

        raise ValueError(f'Cannot find {self.name} in NLL')
    # ----------------------
    def zfit_cons(self, holder : ParsHolder) -> GConstraint | PConstraint:
        '''
        Zfit constraint corresponding to `par`
        '''
        obs = self._obs_from_holder(holder = holder)

        match self.kind:
            case 'GaussianConstraint': 
                cons = GConstraint(
                    params      = obs, 
                    observation = self.observation, 
                    uncertainty = self.sg)
            case 'PoissonConstraint':
                cons = PConstraint(
                    params      = obs, 
                    observation = self.observation)
            case _:
                raise ValueError(f'Invalid constraint kind: {self.kind}')

        return cons
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''
        return f'{self.name:<20}{self.mu:<20}{self.sg:<20}{self.kind:<20}'
    # ----------------------
    def resample(self) -> None:
        '''
        Sets the value of the constrained parameter to
        '''
        match self.kind:
            case 'GaussianConstraint':
                new_val = numpy.random.normal(loc=self.mu, scale=self.sg)
            case 'PoissonConstraint':
                new_val = numpy.random.poisson(lam=self.mu)
            case _:
                raise ValueError(f'Invalid constraint kind: {self.kind}')

        self.observation.set_value(new_val)
# ----------------------------------------
def print_constraints(constraints : list[Constraint1D]) -> None:
    '''
    Parameters
    -------------
    List of constraint objects
    '''
    log.info(f'{"Parameter":<20}{"Value":<20}{"Error":<20}{"Kind":<20}')
    for constraint in constraints:
        log.info(constraint)
# ----------------------------------------
