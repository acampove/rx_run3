'''
This module contains tests for the ConstraintAdder class
'''

import math
import tqdm
import numpy
import pandas as pnd
import pytest

from dmu.stats.zfit import zfit
from typing      import Union
from zfit.loss   import ExtendedUnbinnedNLL, UnbinnedNLL 
from omegaconf   import DictConfig
from dmu.stats   import Constraint1D, ConstraintND
from dmu.stats   import ConstraintAdder
from dmu.stats   import utilities as sut
from dmu.generic import utilities as gut
from dmu.generic import rxran
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
def _extract_parameters(nll : Loss) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    nll : Likelihood, with constraints

    Returns
    -------------
    Pandas series with observed value and parameter name
    '''
    data  = {'Parameter' : [], 'Value' : []}
    s_par = nll.get_params()
    for par in s_par:
        value = float(par.value().numpy())
        data['Parameter'].append(par.name)
        data['Value'    ].append(value)

    df = pnd.DataFrame(data)

    return df
# ----------------------
def _validate(df : pnd.DataFrame, cfg : DictConfig) -> None:
    '''
    Check that for each parameter the mean and std from the dataframe (toys)
    is compatible (within 15%) to what is in the config

    Parameters
    -------------
    df : Dataframe with parameter name, observed value and toy inde
    cfg: Config with user defined information 
    '''

    for tmp, df_par in df.groupby('Parameter'):
        if tmp not in cfg:
            continue

        toy_par   = str(tmp)
        cfg_block = cfg[toy_par]
        log.debug('')
        log.debug(f'Parameter: {toy_par}')

        tmp  = df_par['Value'].mean()
        mean = float(tmp)
        expc = cfg_block.mu

        log.debug(f'Mean: {mean:.2f}/{expc:.2f}')
        assert math.isclose(mean, expc, rel_tol=0.01) # Constrain and measurement should be the same

        if   cfg_block.kind == 'GaussianConstraint':
            expc = cfg_block.sg
        elif cfg_block.kind == 'PoissonConstraint':
            expc = math.sqrt(cfg_block.mu)
        else:
            raise ValueError(f'Invalid constraint: {cfg_block.kind}')

        tmp  = df_par['Value'].std()
        std  = float(tmp)

        log.debug(f'STD : {std:.2f}/{expc:.2f}')
        assert std < expc # Parameters distribution in data should be narrower than constrain
# ----------------------
def test_uncorrelated() -> None:
    '''
    Test multiple uncorrelated constraints
    '''
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_data(package='dmu_data', fpath='tests/stats/constraints/uncorrelated.yaml')
    cons : list[Constraint] = [ Constraint1D(**data) for data in cns.values() ]

    cad = ConstraintAdder(nll=nll, constraints=cons)
    nll = cad.get_nll()
# ----------------------
def test_correlated() -> None:
    '''
    Test constraints of correlated parameters
    '''
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_data(package='dmu_data', fpath='tests/stats/constraints/correlated.yaml')
    cons : list[Constraint] = [ ConstraintND(**data) for data in cns.values() ]

    cad = ConstraintAdder(nll=nll, constraints=cons)
    nll = cad.get_nll()
# ----------------------
@pytest.mark.timeout(100)
def test_toy() -> None:
    '''
    Tests toy constraint addition in a loop
    with timeout extended in order to profile
    '''
    ntoy= 100
    nll = sut.get_nll(kind='s+b', nentries = 100)
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/uncorrelated.yaml')
    cons : list[Constraint] = [ Constraint1D(kind=data.kind, name=data.name, mu=data.mu, sg=data.sg) for data in cns.values() ]

    log.info('')
    print_constraints(cons)

    mnm = zfit.minimize.Minuit()
    cad = ConstraintAdder(nll=nll, constraints=cons)
    nll = cad.get_nll()

    l_df : list[pnd.DataFrame] = []
    sam = nll.data[0]
    for _ in tqdm.trange(ntoy, ascii=' -'):
        cad.resample()
        sam.resample()
        mnm.minimize(nll)

        df = _extract_parameters(nll=nll)
        l_df.append(df)

    df = pnd.concat(l_df, ignore_index=True)

    _validate(df=df, cfg=cns)
# ----------------------
