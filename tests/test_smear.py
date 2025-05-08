'''
Script used to test Q2Smear
'''
from ROOT import RDataFrame, RDF

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from dmu.logging.log_store    import LogStore
from rx_q2.q2smear_calculator import Q2SmearCalculator, WrongQ2SmearInput

log  = LogStore.add_logger('rx_q2:test_q2smear_calculator')
# -------------------------------------------
class Data:
    '''
    Data class
    '''
    nentries = 10_000
    columns  = ['Jpsi_M_brem_track_2', 'nbrem', 'block', 'L1_TRUEID']
# -------------------------------------------
def _plot_masses(rdf : RDataFrame) -> None:
    data = rdf.AsNumpy(['Jpsi_M_brem_track_2', 'Jpsi_M_brem_track_2_smr'])
    df   = pnd.DataFrame(data)

    df.plot.hist(bins=60, histtype='step')
    plt.show()
# -------------------------------------------
def _get_rdf(lepton_id : int = 11, uniform : bool = True) -> RDataFrame:
    df                        = pnd.DataFrame(columns=Data.columns)
    df['nbrem']               = numpy.random.choice([0, 1, 2], Data.nentries)
    df['block']               = numpy.random.choice(range(9) , Data.nentries)
    df['L1_TRUEID']           = lepton_id
    df['EVENTNUMBER']         = 1
    df['RUNNUMBER']           = 1

    if uniform:
        df['Jpsi_M_brem_track_2'] = numpy.random.uniform(0, 22e6 , Data.nentries)
    else:
        df['Jpsi_M_brem_track_2'] = numpy.random.normal(loc=10e6, scale=1e6 , size=Data.nentries)

    rdf = RDF.FromPandas(df)

    return rdf
# -------------------------------------------
def test_wrong_input():
    '''
    Checks if the input is wrong
    '''
    rdf = _get_rdf(lepton_id = 13)

    with pytest.raises(WrongQ2SmearInput):
        obj = Q2SmearCalculator(rdf=rdf)
        rdf = obj.get_rdf()
# -------------------------------------------
def test_simple():
    '''
    Checks if the input is wrong
    '''
    rdf = _get_rdf(lepton_id = 11)
    obj = Q2SmearCalculator(rdf=rdf)
    rdf = obj.get_rdf()

    _plot_masses(rdf)
# -------------------------------------------
