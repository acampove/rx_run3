'''
Module containing constraint classes 
'''

from pydantic import BaseModel

class GaussianConstraint(BaseModel):
    '''
    Class representing Gaussian constrain
    '''
    name: str
    mu  : float
    sg  : float

class PoissonConstraint(BaseModel):
    '''
    Class representing Poisson constrain
    '''
    name : str
    lam  : float
