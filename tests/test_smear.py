'''
Script used to test Q2Smear
'''
import os

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from dmu.logging.log_store    import LogStore
from dmu.generic              import typing_utilities as tut
from rx_q2.q2smear_corrector import Q2SmearCorrector

log  = LogStore.add_logger('rx_q2:test_q2smear_corrector')
# -------------------------------------------
class Data:
    '''
    Data class
    '''
    nentries = 10_000
    out_dir  = '/tmp/tests/rx_q2/smear'

    os.makedirs(out_dir, exist_ok=True)
# -------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_q2:q2smear_corrector'     , 20)
    LogStore.set_level('rx_q2:test_q2smear_corrector', 10)
# -------------------------------------------
def _plot_masses(df : pnd.DataFrame, name : str) -> None:
    log.info('Plotting')

    ax=None
    ax=df.plot.hist(y='reco_mass', range=[2000, 3500], bins=60, alpha=0.3      , ax=ax)
    ax=df.plot.hist(y='mass_smr' , range=[2000, 3500], bins=60, histtype='step', ax=ax)

    plt.savefig(f'{Data.out_dir}/{name}.png')
    plt.close()
# -------------------------------------------
def _get_df(uniform : bool = True) -> pnd.DataFrame:
    df             = pnd.DataFrame()
    df['nbrem']    = numpy.random.choice([0, 1, 2], Data.nentries)
    df['block']    = numpy.random.choice(range(9) , Data.nentries)

    if uniform:
        df['true_mass'] = numpy.random.uniform(1800, 3700, Data.nentries)
        df['reco_mass'] = numpy.random.uniform(1800, 3700, Data.nentries)
    else:
        df['true_mass'] = numpy.random.normal(loc=3000, scale=100, size=Data.nentries)
        df['reco_mass'] = numpy.random.normal(loc=3000, scale=100, size=Data.nentries)

    return df
# -------------------------------------------
@pytest.mark.parametrize('is_uniform', [True, False])
def test_simple(is_uniform : bool):
    '''
    Checks if the input is wrong
    '''
    obj = Q2SmearCorrector()
    df  = _get_df(uniform = is_uniform)

    l_val = []
    itr   = df.itertuples(index=False)
    for nbrem, block, reco_mass, true_mass in tqdm.tqdm(itr, total=len(df), ascii=' -'):
        val    = obj.get_mass(nbrem=nbrem, block=block, jpsi_mass_reco=reco_mass, jpsi_mass_true=true_mass)
        l_val += [val]

    df['mass_smr'] = l_val

    _plot_masses(df=df, name = f'simple_{is_uniform}')
# -------------------------------------------
