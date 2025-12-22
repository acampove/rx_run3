'''
Module containing constraint classes 
'''

from zfit.constraint       import GaussianConstraint as GConstraint
from zfit.constraint       import PoissonConstraint  as PConstraint
from zfit.param            import Parameter as zpar

from pydantic              import BaseModel
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:constraint')
# ----------------------------------------
class GaussianConstraint(BaseModel):
    '''
    Class representing Gaussian constrain
    '''
    name: str
    mu  : float
    sg  : float
    # ---------------------
    @staticmethod
    def from_dict(data : dict[str, tuple[float,float]]) -> list['GaussianConstraint']:
        '''
        Parameters
        ---------------
        data: Dictionary mapping parameter name to tuple with value and error

        Returns
        ---------------
        List of constraint objects
        '''
        constraints = []
        for name, (value, error) in data.items():
            cns = GaussianConstraint(
                name = name,
                mu   = value,
                sg   = error)

            constraints.append(cns)

        return constraints
    # ----------------------
    def to_zfit(self, par : zpar) -> GConstraint:
        '''
        Parameters
        -------------
        par : Zfit parameter

        Returns
        -------------
        Zfit constraint corresponding to `par`
        '''
        return GConstraint(params=par, observation=self.mu, uncertainty=self.sg)
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''
        return f'{self.name:<20}{self.mu:<20}{self.sg:<20}{"Gaussian":<20}'
# ----------------------------------------
class PoissonConstraint(BaseModel):
    '''
    Class representing Poisson constrain
    '''
    name : str
    lam  : float
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''
        return f'{self.name:<20}{self.lam:<20}{"NA":<20}{"Poisson":<20}'
    # ----------------------
    def to_zfit(self, par : zpar) -> PConstraint:
        '''
        Parameters
        -------------
        par : Zfit parameter

        Returns
        -------------
        Zfit constraint corresponding to `par`
        '''
        return PConstraint(params=par, observation=self.lam)
# ----------------------------------------
Constraint = GaussianConstraint | PoissonConstraint
def print_constraints(constraints : list[Constraint]) -> None:
    '''
    Parameters
    -------------
    List of constraint objects
    '''
    log.info(f'{"Parameter":<20}{"Value":<20}{"Error":<20}{"Kind":<20}')
    for constraint in constraints:
        log.info(constraint)
# ----------------------------------------
