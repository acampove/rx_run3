'''
This module contains tests for the ConstraintAdder class
'''

import numpy
from omegaconf import OmegaConf
import pytest
from dmu.stats.constraint_adder import ConstraintAdder
from dmu.stats                  import utilities as sut
from dmu.generic                import utilities as gut

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:test_constraint_adder')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    numpy.random.seed(42)

    LogStore.set_level('dmu:stats:constraint_adder', 10)
# ----------------------
@pytest.mark.parametrize('mode', ['toy', 'real'])
def test_simple(mode : str) -> None:
    '''
    This is the simplest test of ConstraintAdder

    Parameters 
    -------------
    mode : Kind of constraints that will be added
    '''
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/constraint_adder.yaml')

    cad = ConstraintAdder(nll=nll, cns=cns)
    nll = cad.get_nll(mode=mode)

    cov_gauss_out = nll.constraints[0].covariance.numpy()
    cov_gauss_inp = numpy.array(cns.signal_shape.cov)
    assert numpy.isclose(cov_gauss_out, cov_gauss_inp, rtol=1e-5).all()

    obs_gauss_out = numpy.array(nll.constraints[0].observation)
    obs_gauss_inp = numpy.array(cns.signal_shape.observation)

    obs_poiss_out = numpy.array(nll.constraints[1].observation)
    obs_poiss_inp = numpy.array(cns.yields.observation)

    # In toy mode the mean of the normal and lambda of the poisson
    # will be sampled from the constraining PDF and will not be
    # close to the inputs
    if mode == 'real':
        assert numpy.isclose(obs_gauss_out, obs_gauss_inp, rtol=1e-5).all()
        assert numpy.isclose(obs_poiss_out, obs_poiss_inp, rtol=1e-5).all()
    else:
        assert not numpy.isclose(obs_gauss_out, obs_gauss_inp, rtol=1e-5).any()
        assert not numpy.isclose(obs_poiss_out, obs_poiss_inp, rtol=1e-5).any()
# ----------------------
@pytest.mark.parametrize('kind', ['GaussianConstraint', 'PoissonConstraint'])
def test_dict_to_const(kind : str) -> None:
    '''
    This tests utility that converts python dictionary to
    DictConfig used to hold constraints
    '''
    d_cns = {
        'a' : (0., 1.),
        'b' : (5., 2.),
    }    

    # TODO: Improve test with assertions
    cns = ConstraintAdder.dict_to_cons(d_cns=d_cns, name='test', kind=kind)
    log.info('\n\n' + OmegaConf.to_yaml(cns))
