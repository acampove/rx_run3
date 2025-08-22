'''
This module contains tests for the ConstraintAdder class
'''
from typing import Union

import math
import zfit
import tqdm
import numpy
import pandas as pnd
import pytest
from zfit.loss                  import ExtendedUnbinnedNLL, UnbinnedNLL 
from omegaconf                  import OmegaConf, DictConfig
from dmu.stats.constraint_adder import ConstraintAdder
from dmu.stats                  import utilities as sut
from dmu.generic                import utilities as gut
from dmu.logging.log_store      import LogStore

log=LogStore.add_logger('dmu:stats:test_constraint_adder')
Loss=Union[ExtendedUnbinnedNLL,UnbinnedNLL]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    numpy.random.seed(42)

    LogStore.set_level('dmu:stats:constraint_adder'     , 10)
    LogStore.set_level('dmu:stats:test_constraint_adder', 10)
# ----------------------
def _extract_observables(nll : Loss) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    nll : Likelihood, with constraints

    Returns
    -------------
    Pandas series with observed value, parameter name and toy index
    '''
    df = pnd.DataFrame(columns=['Parameter', 'Value'])

    l_val = []
    l_nam = []
    for constraint in nll.constraints:
        l_obs = constraint.observation
        l_val+= [ obs.value().numpy()           for obs in l_obs ]
        l_nam+= [ obs.name.removesuffix('_cns') for obs in l_obs ]

    df['Parameter'] = l_nam
    df['Value'    ] = l_val

    return df
# ----------------------
def _validate(df : pnd.DataFrame, cfg : DictConfig) -> None:
    '''
    Parameters
    -------------
    df : Dataframe with parameter name, observed value and toy inde
    cfg: Config with user defined information 
    '''
    for par, df_par in df.groupby('Parameter'):
        for kind, cfg_block in cfg.items():
            log.debug(f'Kind: {kind}')

            l_par = cfg_block.parameters
            if par not in l_par:
                continue

            l_obs = cfg_block.observation
            if kind == 'signal_shape':
                mat   = cfg_block.cov
                l_var = numpy.diag(mat)
            else:
                l_var = l_obs

            d_par_obs = dict(zip(l_par, l_obs))
            d_par_var = dict(zip(l_par, l_var))

            tmp  = df_par['Value'].mean()
            mean = float(tmp)
            expc = d_par_obs[par]
            log.debug(f'Mean: {mean:.0f}/{expc}')
            assert math.isclose(mean, expc, rel_tol=0.15) 

            tmp  = df_par['Value'].var()
            mean = float(tmp)
            expc = d_par_var[par]

            log.debug(f'Variance: {mean:.0f}/{expc}')
            assert math.isclose(mean, expc, rel_tol=0.15) 
# ----------------------
def test_simple() -> None:
    '''
    This is the simplest test of ConstraintAdder
    '''
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/constraint_adder.yaml')

    cad = ConstraintAdder(nll=nll, cns=cns)
    nll = cad.get_nll()

    cov_gauss_out = nll.constraints[0].covariance.numpy()
    cov_gauss_inp = numpy.array(cns.signal_shape.cov)

    obs_gauss_out = numpy.array(nll.constraints[0].observation)
    obs_gauss_inp = numpy.array(cns.signal_shape.observation)

    obs_poiss_out = numpy.array(nll.constraints[1].observation)
    obs_poiss_inp = numpy.array(cns.yields.observation)

    assert numpy.isclose(cov_gauss_out, cov_gauss_inp, rtol=1e-5).all()
    assert numpy.isclose(obs_gauss_out, obs_gauss_inp, rtol=1e-5).all()
    assert numpy.isclose(obs_poiss_out, obs_poiss_inp, rtol=1e-5).all()
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
# ----------------------
@pytest.mark.timeout(100)
def test_toy() -> None:
    '''
    Tests toy constraint addition in a loop
    with timeout extended in order to profile
    '''
    ntoy= 100
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/constraint_adder.yaml')
    sam = nll.data[0]

    mnm = zfit.minimize.Minuit()
    cad = ConstraintAdder(nll=nll, cns=cns)
    nll = cad.get_nll()

    for _ in tqdm.trange(ntoy, ascii=' -'):
        cad.resample()
        sam.resample()
        mnm.minimize(nll)
