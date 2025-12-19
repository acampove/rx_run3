'''
Module containing constraint classes 
'''

from pydantic import BaseModel
from dmu      import LogStore

log=LogStore.add_logger('fitter::constraint')
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
