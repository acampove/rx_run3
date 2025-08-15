'''
Module with functions meant to test the ToyPlotter class
'''
from functools import lru_cache

import pytest
import numpy
import pandas as pnd

from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

from fitter.toy_plotter    import ToyPlotter

log=LogStore.add_logger('fitter:test_toy_plotter')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:plotting:Plotter1D', 10)
# ----------------------
@lru_cache(maxsize=3)
def _get_df(ntoys : int, l_var : tuple[str]) -> pnd.DataFrame:
    '''
    Returns 
    ------------------
    dataframe with fake fit parameter information
    '''
    numpy.random.seed(42)
    l_df_var = []
    for var in l_var:
        df_var              = pnd.DataFrame(columns=['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Converged'])
        df_var['Parameter'] = ntoys * [var]
        if var != 'b':
            df_var['Gen'      ] = numpy.zeros(ntoys)
            df_var['Value'    ] = numpy.random.normal(loc=0.0, scale=1.0, size=ntoys)
            df_var['Error'    ] = numpy.random.normal(loc=1.0, scale=0.1, size=ntoys)
        else:
            df_var['Gen'      ] = numpy.array(ntoys * [1000])
            df_var['Value'    ] = numpy.random.poisson(lam=1000, size=ntoys)
            df_var['Error'    ] = numpy.random.normal(loc=30, scale=10, size=ntoys)
        df_var['Toy'      ] = numpy.arange(ntoys) 

        l_df_var.append(df_var)

    df = pnd.concat(l_df_var)
    df = df.reset_index(drop=True)

    npar = len(l_var)
    for itoy in range(ntoys):
        gof = numpy.random.uniform(low=0, high=1, size=1)
        cnv = numpy.random.choice(a=[True, False], p=[0.95, 0.05], size=1)

        df.loc[df.Toy == itoy,       'GOF'] = npar * [gof]
        df.loc[df.Toy == itoy, 'Converged'] = npar * [cnv]

    df['GOF']       = df['GOF'      ].astype(float)
    df['Converged'] = df['Converged'].astype(bool )

    for numeric in ['Value', 'Error', 'Gen', 'Toy', 'GOF']:
        log.info(f'Checking: {numeric}')
        assert numpy.isfinite(df[numeric]).all()

    return df
# ----------------------
def test_simple() -> None:
    '''
    This is the simplest test of ToyPlotter
    '''
    log.info('')
    df = _get_df(ntoys=1000, l_var=('a', 'b'))
    cfg= gut.load_conf(package='fitter_data', fpath='toys/test.yaml')

    ptr= ToyPlotter(df=df, cfg=cfg)
    ptr.plot()
    
