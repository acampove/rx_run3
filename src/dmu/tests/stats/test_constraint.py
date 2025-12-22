'''
Module with tests for classes and functions in constraint.py module
'''
import zfit
import numpy

from zfit.constraint import GaussianConstraint as GConstraint 
from zfit.constraint import PoissonConstraint  as PConstraint 

from dmu.stats import GaussianConstraint
from dmu.stats import PoissonConstraint 
from dmu.stats import print_constraints 

# ---------------------------------
def test_gauss():
    '''
    Basic test for GaussianConstraint
    '''
    numpy.random.seed(42)

    par = zfit.param.Parameter('a', 1, 0, 2)
    cns = GaussianConstraint(
        par = par, 
        mu  = 1, 
        sg  = 2)

    assert isinstance(cns.zfit_cons, GConstraint)

    values = []
    for _ in range(100):
        cns.resample()
        val = cns.observation.value()
        values.append(val)

    mu=numpy.mean(values)
    sg=numpy.std(values)

    assert numpy.isclose(mu, 0.792306965211812, rtol=1e-5)
    assert numpy.isclose(sg, 1.807232353289259, rtol=1e-5)
# ---------------------------------
def test_poisson():
    '''
    Basic test for PoissonConstraint
    '''
    numpy.random.seed(42)

    par = zfit.param.Parameter('a', 1, 0, 20)
    cns = PoissonConstraint(
        par = par, 
        lam = 10.)

    assert isinstance(cns.zfit_cons, PConstraint)

    values = []
    for _ in range(100):
        cns.resample()
        val = cns.observation.value()
        values.append(val)

    lam = numpy.mean(values)

    assert numpy.isclose(lam, 10.06, rtol=1e-5)
# ---------------------------------
def test_print_constraint():
    '''
    Test printing of constraints
    '''
    par = zfit.param.Parameter('a', 1, 0, 2)
    ca  = PoissonConstraint(
        par = par, 
        lam = 1)

    par = zfit.param.Parameter('a', 1, 0, 20)
    cb  = GaussianConstraint(
        par = par, 
        mu  = 1, 
        sg  = 2)

    print_constraints(constraints = [ca, cb])
