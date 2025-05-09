'''
Script used to test Q2Smear
'''
import os

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from ROOT                     import RDataFrame, RDF
from dmu.logging.log_store    import LogStore
from rx_q2.q2smear_corrector import Q2SmearCorrector, WrongQ2SmearInput

log  = LogStore.add_logger('rx_q2:test_q2smear_corrector')
# -------------------------------------------
class Data:
    '''
    Data class
    '''
    nentries = 10_000
    columns  = ['Jpsi_M_brem_track_2', 'nbrem', 'block', 'L1_TRUEID']
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
def _get_rdf(lepton_id : int = 11, uniform : bool = True) -> RDataFrame:
    df                        = pnd.DataFrame(columns=Data.columns)
    df['nbrem']               = numpy.random.choice([0, 1, 2], Data.nentries)
    df['block']               = numpy.random.choice(range(9) , Data.nentries)
    df['L1_TRUEID']           = lepton_id
    df['EVENTNUMBER']         = 1
    df['RUNNUMBER']           = 1

    if uniform:
        df['Jpsi_M_brem_track_2'] = numpy.random.uniform(2000, 3500, Data.nentries)
    else:
        df['Jpsi_M_brem_track_2'] = numpy.random.normal(loc=3000, scale=40, size=Data.nentries)

    rdf = RDF.FromPandas(df)

    return rdf
# -------------------------------------------
def test_wrong_input():
    '''
    Checks if the input is wrong
    '''
    rdf = _get_rdf(lepton_id = 13)

    with pytest.raises(WrongQ2SmearInput):
        obj = Q2SmearCorrector(rdf=rdf)
        rdf = obj.get_rdf()
# -------------------------------------------
@pytest.mark.parametrize('is_uniform', [True, False])
def test_simple(is_uniform : bool):
    '''
    Checks if the input is wrong
    '''
    rdf = _get_rdf(lepton_id = 11, uniform = is_uniform)
    obj = Q2SmearCorrector(rdf=rdf)
    rdf = obj.get_rdf()

    _plot_masses(rdf, name = f'simple_{is_uniform}')
# -------------------------------------------
