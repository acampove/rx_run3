'''
This module contains tests for the ConstraintAdder class
'''

import math
import zfit
import tqdm
import numpy
import pandas as pnd
import pytest

from typing      import Union
from rx_common   import rxran
from zfit.loss   import ExtendedUnbinnedNLL, UnbinnedNLL 
from omegaconf   import DictConfig
from dmu.stats   import Constraint1D, ConstraintND
from dmu.stats   import ConstraintAdder
from dmu.stats   import utilities as sut
from dmu.generic import utilities as gut
from dmu         import LogStore
from dmu.stats.constraint import print_constraints

log        = LogStore.add_logger('dmu:stats:test_constraint_adder')
Loss       = Union[ExtendedUnbinnedNLL, UnbinnedNLL]
Constraint = Union[Constraint1D, ConstraintND]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:stats:constraint_adder'     , 10)
    LogStore.set_level('dmu:stats:test_constraint_adder', 10)

    with rxran.seed(42):
        yield
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
            mean = float(tmp) # type:ignore
            expc = d_par_var[par]

            log.debug(f'Variance: {mean:.0f}/{expc}')
            assert math.isclose(mean, expc, rel_tol=0.15) 
# ----------------------
def test_uncorrelated() -> None:
    '''
    This is the simplest test of ConstraintAdder
    '''
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/uncorrelated.yaml')
    cons : list[Constraint] = [ Constraint1D(kind=data.kind, name=data.name, mu=data.mu, sg=data.sg) for data in cns.values() ]

    cad = ConstraintAdder(nll=nll, constraints=cons)
    nll = cad.get_nll()
# ----------------------
@pytest.mark.timeout(100)
def test_toy() -> None:
    '''
    Tests toy constraint addition in a loop
    with timeout extended in order to profile
    '''
    ntoy= 400
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/uncorrelated.yaml')
    cons : list[Constraint] = [ Constraint1D(kind=data.kind, name=data.name, mu=data.mu, sg=data.sg) for data in cns.values() ]

    sam = nll.data[0]

    mnm = zfit.minimize.Minuit()
    cad = ConstraintAdder(nll=nll, constraints=cons)
    nll = cad.get_nll()

    l_df = []
    for _ in tqdm.trange(ntoy, ascii=' -'):
        cad.resample()
        sam.resample()
        mnm.minimize(nll)

        df = _extract_observables(nll=nll)
        l_df.append(df)

    df = pnd.concat(l_df)
    _validate(df=df, cfg=cns)
