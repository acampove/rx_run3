'''
Module containing constraint classes 
'''

import zfit
import math
import numpy

from typing                import Sequence
from tabulate              import tabulate
from typing                import Protocol
from functools             import cached_property
from zfit.constraint       import GaussianConstraint as GConstraint
from zfit.constraint       import PoissonConstraint  as PConstraint
from zfit.param            import Parameter as zpar
from zfit.result           import FitResult

from pydantic              import BaseModel, model_validator, TypeAdapter
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
class Constraint:
    '''
    Class with common code to 1D and ND constraints
    '''
    # ----------------------
    def _get_parameter_value(self, name : str, result : FitResult) -> float:
        '''
        Parameters
        -------------
        name  : Name of parameter to be calibrated
        holder: Object with `get_params` implemented

        Returns
        -------------
        New value of parameters constraint mean
        '''
        for par in result.params:
            if par.name != name:
                continue

            return float(par.value().numpy())

        raise ValueError(f'Parameter {name} not found in holder')
    # ----------------------
    def calibrate(self, result : FitResult) -> 'Constraint':
        _ = result

        raise NotImplementedError('Cannot calibrate base Constraint')
    # ----------------------
    def resample(self) -> None:
        raise NotImplementedError('Cannot resample base Constraint')
# ----------------------------------------
class ConstraintND(BaseModel, Constraint):
    '''
    Class meant to symbolize NDimensional Gaussian constraint
    '''
    kind       : str
    parameters : list[str]
    values     : list[float]
    cov        : list[list[float]]
    # ----------------------
    def calibrate(self, result : FitResult) -> 'ConstraintND':
        '''
        Re-centers mu values of this constraint to value of parameter in holder.
        Needed before fitting toys. The constraints need to be consistent with the
        parameter values used to generate toy data.

        Parameters
        -------------
        result: Object holding result of fit

        Returns
        -------------
        Copy of this constraint with means calibrated
        '''
        new_values = [ self._get_parameter_value(name = name, result = result ) for name in self.parameters ]

        return ConstraintND(
            kind       = self.kind,
            parameters = self.parameters,
            values     = new_values, 
            cov        = self.cov,
        )
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation of constrint
        '''
        parameters = [ f'{name:<30}{value:<20.3f}' for name, value in zip(self.parameters, self.values) ]

        msg  = '\n'
        msg += '\n'
        msg += 'Covariance:\n'
        msg += tabulate(self.cov, tablefmt="grid") + '\n'
        msg += '\n'
        msg += 'Parameters\n'
        msg += '---------------\n'
        msg += '\n'.join(parameters)
        
        return msg
    # ---------------------
    @model_validator(mode='after')
    def check_dimensions(self) -> 'ConstraintND':
        '''
        Checks that all the inputs are of the same dimension
        '''
        dimension = len(self.parameters)
        
        if len(self.values) != dimension:
            raise ValueError(f"observation length ({len(self.values)}) must match parameters length ({dimension})")
        
        if len(self.cov) != dimension:
            raise ValueError(f"cov must have {dimension} rows, but has {len(self.cov)}")
            
        for irow, row in enumerate(self.cov):
            if len(row) != dimension:
                raise ValueError(f"cov row {irow} must have {dimension} columns, but has {len(row)}")
        
        return self
    # ----------------------
    def _obs_from_holder(self, holder : ParsHolder) -> list[zpar]:
        '''
        Parameters
        ------------------
        holder: Object holding parameters

        Returns
        ------------------
        Observables of constraint, i.e. parameters to constrain
        '''
        s_par = holder.get_params()

        obs : list[zpar] = []
        for par in s_par:
            if par.name not in self.parameters: 
                continue

            obs.append(par)

        if not obs:
            raise ValueError('No observable found')

        return obs
    # ----------------------
    @cached_property
    def observations(self) -> dict[str,zpar]:
        '''
        Returns
        -------------
        List with parameters synbolizing the mean of the ND Gaussian
        '''
        d_obs : dict[str,zpar] = dict()
        for ipar, name in enumerate(self.parameters):
            value = self.values[ipar]
            error = math.sqrt(self.cov[ipar][ipar])

            obs = zfit.Parameter(
                f'{name}_mu', 
                value, 
                value - 5 * error, 
                value + 5 * error)

            d_obs[name] = obs

        return d_obs
    # ---------------------
    def zfit_cons(self, holder : ParsHolder) -> GConstraint:
        '''
        Parameters
        ------------
        holder: Object holding zfit parameters, e.g. Likelihood

        Returns
        ------------
        Multidimentional Gaussian constraint
        '''
        l_par = self._obs_from_holder(holder = holder)
        l_obs = self.observations.values()

        cns   = zfit.constraint.GaussianConstraint(
            params      = l_par, 
            observation = l_obs,
            cov         = self.cov)

        return cns
    # ----------------------
    def observation(self, name : str) -> float:
        '''
        Parameters
        -------------
        name: Name of parameter that will be constrained

        Returns
        -------------
        Value to which it will be constrained
        '''
        if name not in self.observations:
            raise ValueError(f'Parameter {name} not found')

        par = self.observations[name]

        return float(par.value().numpy())
    # ----------------------
    def resample(self) -> None:
        '''
        Sets the value of the constrained parameter to
        '''
        new_values = numpy.random.multivariate_normal(self.values, self.cov, size=1)
        for new_value, observation in zip(new_values[0], self.observations.values()):
            observation.set_value(new_value)
# ----------------------------------------
class Constraint1D(BaseModel, Constraint):
    '''
    Class representing Gaussian 1D constrain
    '''
    kind: str
    name: str
    mu  : float
    sg  : float
    # ----------------------
    @classmethod
    def from_dict(
        cls,
        data : dict[str,tuple[float,float]],
        kind : str) -> list['Constraint1D']:
        '''
        Parameters
        -----------------
        data: Dictionary storing parameter names, and tuples with measurements and errors
        kind: String specifying type of constraint

        Returns
        -----------------
        List of 1D constraints
        '''

        constraints = []
        for name, (value, error) in data.items():
            cns = cls(
                kind = kind,
                name = name,
                mu   = value,
                sg   = error)

            constraints.append(cns)

        return constraints
    # ----------------------
    def calibrate(self, result : FitResult) -> 'Constraint1D':
        '''
        Re-centers mu values of this constraint to value of parameter in holder.
        Needed before fitting toys. The constraints need to be consistent with the
        parameter values used to generate toy data.

        Parameters
        -------------
        result: Object holding result of fit 

        Returns
        -------------
        Copy of this constraint with means calibrated
        '''

        return Constraint1D(
            kind   = self.kind,
            name   = self.name,
            mu     = self._get_parameter_value(name = self.name, result = result), 
            sg     = self.sg,
        )
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
def build_constraint(data: dict) -> Constraint:
    '''
    Parameters
    ---------------
    data: Python dictionary with constraint information

    Returns
    ---------------
    Constraint object
    '''
    adapter = TypeAdapter(Constraint1D | ConstraintND)

    try:
        return adapter.validate_python(data)
    except Exception as exc:
        raise ValueError('Cannot build constrain from input') from exc
# ----------------------------------------
def print_constraints(constraints : Sequence[Constraint]) -> None:
    '''
    Parameters
    -------------
    List of constraint objects
    '''
    l_cons_1d = [ cons for cons in constraints if isinstance(cons, Constraint1D) ]
    l_cons_nd = [ cons for cons in constraints if isinstance(cons, ConstraintND) ]

    if l_cons_1d:
        _print_1d_constraints(l_cons_1d)

    if l_cons_nd:
        _print_nd_constraints(l_cons_nd)
# ----------------------------------------
def _print_1d_constraints(constraints : Sequence[Constraint1D]) -> None:
    '''
    Prints list of constraints
    '''
    log.info(f'{"Parameter":<20}{"Value":<20}{"Error":<20}{"Kind":<20}')
    log.info(80 * '-')
    for constraint in constraints:
        log.info(constraint)
# ----------------------------------------
def _print_nd_constraints(constraints : Sequence[ConstraintND]) -> None:
    '''
    Prints list of constraints
    '''
    for constraint in constraints:
        log.info(constraint)
# ----------------------------------------
