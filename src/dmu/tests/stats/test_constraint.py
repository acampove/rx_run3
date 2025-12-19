'''
Module with tests for classes and functions in constraint.py module
'''

from dmu.stats import GaussianConstraint
from dmu.stats import PoissonConstraint 
from dmu.stats import print_constraints 

def test_make_gauss():
    '''
    Basic test for GaussianConstraint
    '''
    cns = GaussianConstraint(
        name = 'a', 
        mu   = 1, 
        sg   = 2)


