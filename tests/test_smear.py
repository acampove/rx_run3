'''
Script used to test Q2Smear
'''
import os

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from dmu.logging.log_store    import LogStore
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
def _plot_masses(rdf : RDataFrame, name : str) -> None:
    data = rdf.AsNumpy(['Jpsi_M_brem_track_2', 'Jpsi_M_brem_track_2_smr'])
    df   = pnd.DataFrame(data)

    df.plot.hist(bins=60, histtype='step')
    plt.savefig(f'{Data.out_dir}/{name}.png')
    plt.close()
# -------------------------------------------
def _get_df(uniform : bool = True) -> pnd.DataFrame:
    df             = pnd.DataFrame(columns=['nbrem', 'block', 'mass'])
    df['nbrem']    = numpy.random.choice([0, 1, 2], Data.nentries)
    df['block']    = numpy.random.choice(range(9) , Data.nentries)

    if uniform:
        df['mass'] = numpy.random.uniform(2000, 3500, Data.nentries)
    else:
        df['mass'] = numpy.random.normal(loc=3000, scale=40, size=Data.nentries)

    return df
# -------------------------------------------
@pytest.mark.parametrize('is_uniform', [True, False])
def test_simple(is_uniform : bool):
    '''
    Checks if the input is wrong
    '''
    obj = Q2SmearCorrector()
    df  = _get_df(uniform = is_uniform)

    for nbrem, block, mass in df.itertuples(index=False):
        val = obj.get_mass(nbrem=nbrem, block=block, jpsi_mass_reco=mass)

    #_plot_masses(rdf, name = f'simple_{is_uniform}')
# -------------------------------------------
