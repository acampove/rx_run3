'''
Script used to test Q2Smear
'''

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                     import RDF
from pathlib                  import Path
from dmu.logging.log_store    import LogStore
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
    rdf      : RDF.RNode, 
    particle : str,
    dir_path : Path) -> None:
    log.info('Plotting')

    data = rdf.AsNumpy()

    arr_reco = data[f'{particle}_M_brem_track_2']
    arr_true = data[f'{particle}_Mass'          ]

    plt.hist(arr_reco, range=(2000, 3500), bins=60, alpha=0.3      )
    plt.hist(arr_true, range=(2000, 3500), bins=60, histtype='step')

    plt.savefig(dir_path / f'{particle}.png')
    plt.close()
# -------------------------------------------
def _get_df(uniform : bool, channel : str) -> pnd.DataFrame:
    df = pnd.DataFrame()

    if   channel == 'ee':
        df['nbrem'] = numpy.random.choice([0, 1, 2], Data.nentries)
    elif channel == 'mm':
        df['nbrem'] = numpy.random.choice([0]      , Data.nentries)
    else:
        raise ValueError(f'Invalid channel: {channel}')

    df['block']    = numpy.random.choice(range(1, 7) , Data.nentries)

    df = _add_mass(df=df, particle='B'   , uniform=uniform)
    df = _add_mass(df=df, particle='Jpsi', uniform=uniform)

    return df
# ----------------------
def _add_mass(
    df       : pnd.DataFrame,
    uniform  : bool,
    particle : str) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    particle: Name of particle to add to dataframe, e.g Jpsi

    Returns
    -------------
    DataFrame with column added
    '''
    
    if uniform:
        df[f'{particle}_M_brem_track_2'] = numpy.random.uniform(1800, 3700, Data.nentries)
        df[f'{particle}_TRUEID'        ] = numpy.random.uniform(1800, 3700, Data.nentries)
    else:
        df[f'{particle}_M_brem_track_2'] = numpy.random.normal(loc=3000, scale=100, size=Data.nentries)
        df[f'{particle}_TRUEID'        ] = numpy.random.normal(loc=3000, scale=100, size=Data.nentries)

    return df
# -------------------------------------------
@pytest.mark.parametrize('is_uniform', [True, False])
@pytest.mark.parametrize('channel'   , ['ee',  'mm'])
def test_get_rdf(
    is_uniform : bool, 
    channel    : str,
    tmp_path):
    '''
    Checks if the input is wrong
    '''
    df  = _get_df(uniform = is_uniform, channel = channel)
    rdf = RDF.FromPandas(df)

    obj = Q2SmearCorrector(channel=channel)
    rdf = obj.get_rdf(rdf=rdf)

    _plot_masses(rdf=rdf, particle='B'   , dir_path=tmp_path)
    _plot_masses(rdf=rdf, particle='Jpsi', dir_path=tmp_path)
# -------------------------------------------
