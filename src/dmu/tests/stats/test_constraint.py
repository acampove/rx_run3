'''
Module with tests for classes and functions in constraint.py module
'''
import ROOT # type: ignore
from dmu.stats.zfit import zfit

from zfit.constraint import GaussianConstraint as GConstraint 
from zfit.constraint import PoissonConstraint  as PConstraint 

from dmu.stats import GaussianConstraint
from dmu.stats import PoissonConstraint 
from dmu.stats import print_constraints 

# ---------------------------------
def test_make_gauss():
    '''
    Basic test for GaussianConstraint
    '''
    cns = GaussianConstraint(
        name = 'a', 
        mu   = 1, 
        sg   = 2)

    par = zfit.param.Parameter('a', 0, 0, 2)
    val = cns.to_zfit(par = par)

    assert isinstance(val, GConstraint)
# ---------------------------------
def test_make_poisson():
    '''
    Basic test for PoissonConstraint
    '''
    cns = PoissonConstraint(
        name = 'a', 
        lam  = 1)

    par = zfit.param.Parameter('a', 0, 0, 2)
    val = cns.to_zfit(par = par)

    assert isinstance(val, PConstraint)
# ---------------------------------
def test_print_constraint():
    '''
    Test printing of constraints
    '''
    ca = PoissonConstraint(
        name = 'a', 
        lam  = 1)

    cb = GaussianConstraint(
        name = 'b', 
        mu   = 1, 
        sg   = 2)

    print_constraints(constraints = [ca, cb])
