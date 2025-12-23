'''
Module with tests for classes and functions in constraint.py module
'''
import zfit
import numpy
import pytest

from typing          import Final
from zfit            import Parameter          as zpar
from zfit.constraint import GaussianConstraint as GConstraint 
from zfit.constraint import PoissonConstraint  as PConstraint 

from dmu.generic import utilities as gut
from dmu.stats   import Constraint1D 
from dmu.stats   import ConstraintND
from dmu.stats   import print_constraints 

from dmu import LogStore

log        = LogStore.add_logger('dmu::test_constraint')
Constraint = Constraint1D | ConstraintND
# ----------------------
_EXPECTED_MU : Final[list[float]] = [
    5078.961534826059,
    9.792306965211813,
    1001.32,
    1001.32,
]
_EXPECTED_SG : Final[list[float]] = [
    9.036161766446284,
    1.807232353289259,
    31.62811407592934,
    31.62811407592934,
]
# ---------------------------------
def _load_constraints(kind = 'uncorrelated') -> list[dict]:
    '''
    Parameters
    -------------
    kind: Type of constraints

    Returns
    -------------
    List of dictionary with information
    '''
    raw_input : dict[str,dict] = gut.load_data(package='dmu_data', fpath=f'tests/stats/constraints/{kind}.yaml')

    data : list[dict] = [] 
    for name, entry in raw_input.items():
        entry['name'] = name
        data.append(entry)

    return data
# ---------------------------------
class ParsHolder:
    '''
    Class representing object holding parameters
    '''
    # ------------
    def get_params(self) -> set[zpar]:
        '''
        Returns set of parameters
        '''
        mu   = zfit.Parameter('mu', 5300, 5000, 6000)
        sg   = zfit.Parameter('sg',   20,   10,   60)

        nsig = zfit.Parameter('nsig', 1000, 500, 2000)
        nbkg = zfit.Parameter('nbkg', 1000, 500, 2000)

        return {mu, sg, nsig, nbkg}
# ---------------------------------
@pytest.mark.parametrize('data, mu, sg', zip(_load_constraints(), _EXPECTED_MU, _EXPECTED_SG))
def test_constraint1D(data : dict, mu : float, sg : float):
    '''
    Basic test for Constraint1D 
    '''
    numpy.random.seed(42)
    holder = ParsHolder()
    cons   = Constraint1D(**data)

    zcons  = cons.zfit_cons(holder)
    assert isinstance(zcons, (GConstraint, PConstraint))

    values = []
    for _ in range(100):
        cons.resample()
        val = cons.observation.value()
        values.append(val)

    mu_test = numpy.mean(values)
    sg_test = numpy.std(values)

    assert numpy.isclose(mu, mu_test, rtol=1e-5)
    assert numpy.isclose(sg, sg_test, rtol=1e-5)
# ---------------------------------
def test_print_1d():
    '''
    Test printing of constraints
    '''
    numpy.random.seed(42)
    l_cons : list[Constraint] = [ Constraint1D(**data) for data in _load_constraints() ]

    log.info('')
    print_constraints(l_cons)
# ---------------------------------
def test_print_nd():
    '''
    Test printing of constraints
    '''
    numpy.random.seed(42)
    l_cons : list[Constraint] = [ ConstraintND(**data) for data in _load_constraints(kind = 'correlated') ]

    log.info('')
    print_constraints(l_cons)
# ---------------------------------
def test_constraintND():
    numpy.random.seed(42)

    data = gut.load_data(package='dmu_data', fpath='tests/stats/constraints/correlated.yaml')
    block= data['signal_shape']

    holder = ParsHolder()
    cons   = ConstraintND(**block)
    zcons  = cons.zfit_cons(holder)

    assert isinstance(zcons, GConstraint)

    l_mu = []
    l_sg = []
    for _ in range(1000):
        cons.resample()
        mu = cons.observation('mu')
        sg = cons.observation('sg')

        l_mu.append(mu)
        l_sg.append(sg)

    mu_mu = numpy.mean(l_mu)
    mu_sg = numpy.mean(l_sg)
    sg_mu = numpy.std(l_mu)
    sg_sg = numpy.std(l_sg)

    assert numpy.isclose(mu_mu , 5079.658, rtol=1e-5)
    assert numpy.isclose(mu_sg , 10.06348, rtol=1e-5)
    assert numpy.isclose(sg_mu , 9.611858, rtol=1e-5)
    assert numpy.isclose(sg_sg , 1.998920, rtol=1e-5)
