'''
Script used to test Q2Smear
'''

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from pathlib                  import Path
from dmu.logging.log_store    import LogStore
from dmu.generic              import typing_utilities as tut
from rx_q2.q2smear_corrector  import Q2SmearCorrector

log  = LogStore.add_logger('rx_q2:test_q2smear_corrector')
# -------------------------------------------
class Data:
    '''
    Data class
    '''
    nentries = 10_000
# -------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_q2:q2smear_corrector'     , 20)
    LogStore.set_level('rx_q2:test_q2smear_corrector', 10)
# -------------------------------------------
def _plot_masses(
    df       : pnd.DataFrame, 
    dir_path : Path,
    name     : str) -> None:
    log.info('Plotting')

    ax=None
    ax=df.plot.hist(y='reco_mass', range=[2000, 3500], bins=60, alpha=0.3      , ax=ax)
    ax=df.plot.hist(y='mass_smr' , range=[2000, 3500], bins=60, histtype='step', ax=ax)

    plt.savefig(dir_path / f'{name}.png')
    plt.close()
# -------------------------------------------
def _get_df(uniform : bool, channel : str) -> pnd.DataFrame:
    df             = pnd.DataFrame()
    if   channel == 'ee':
        df['nbrem'] = numpy.random.choice([0, 1, 2], Data.nentries)
    elif channel == 'mm':
        df['nbrem'] = numpy.random.choice([0]      , Data.nentries)
    else:
        raise ValueError(f'Invalid channel: {channel}')

    df['block']    = numpy.random.choice(range(1, 7) , Data.nentries)

    if uniform:
        df['true_mass'] = numpy.random.uniform(1800, 3700, Data.nentries)
        df['reco_mass'] = numpy.random.uniform(1800, 3700, Data.nentries)
    else:
        df['true_mass'] = numpy.random.normal(loc=3000, scale=100, size=Data.nentries)
        df['reco_mass'] = numpy.random.normal(loc=3000, scale=100, size=Data.nentries)

    return df
# ----------------------
def _correct_q2(row : pnd.Series, corrector : Q2SmearCorrector) -> float:
    '''
    Parameters
    -------------
    row: Pandas series with entry

    Returns
    -------------
    Instance of Q2SmearCorrector
    '''
    args                   = dict()
    args['nbrem']          = tut.numeric_from_series(row, 'nbrem'    ,   int)
    args['block']          = tut.numeric_from_series(row, 'block'    ,   int)
    args['jpsi_mass_true'] = tut.numeric_from_series(row, 'true_mass', float)
    args['jpsi_mass_reco'] = tut.numeric_from_series(row, 'reco_mass', float)

    return corrector.get_mass(**args)
# -------------------------------------------
@pytest.mark.parametrize('is_uniform', [True, False])
@pytest.mark.parametrize('channel'   , ['ee',  'mm'])
def test_simple(is_uniform : bool, channel : str, tmp_path):
    '''
    Checks if the input is wrong
    '''
    obj            = Q2SmearCorrector(channel=channel)
    df             = _get_df(uniform = is_uniform, channel = channel)
    df['mass_smr'] = df.apply(_correct_q2, args=(obj,), axis=1)

    _plot_masses(df=df, name = f'simple_{is_uniform}', dir_path=tmp_path)
# -------------------------------------------
