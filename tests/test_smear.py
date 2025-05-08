'''
Script used to test Q2Smear 
'''
from ROOT import RDataFrame, RDF

import numpy
import pandas as pnd
from dmu.logging.log_store    import LogStore
#from rx_q2.q2smear_calculator import Q2SmearCalculator

log  = LogStore.add_logger('rx_q2:test_q2smear_calculator')
# -------------------------------------------
class Data:
    '''
    Data class
    '''
    nentries = 10_000
# -------------------------------------------
def _get_rdf() -> RDataFrame:
    df                        = pnd.DataFrame(columns=['Jpsi_M_brem_track_2', 'nbrem', 'block'])
    df['Jpsi_M_brem_track_2'] = numpy.random.uniform(0, 22e6 , Data.nentries)
    df['nbrem']               = numpy.random.choice([0, 1, 2], Data.nentries)
    df['block']               = numpy.random.choice(range(9) , Data.nentries)

    rdf = RDF.FromPandas(df)

    return rdf
# -------------------------------------------
def test_simple():
    '''
    Simples test for smearing
    '''
    rdf = _get_rdf()

    rdf.Display().Print()

    return

    obj = Q2SmearCalculator(rdf=rdf)
    rdf = obj.get_rdf()
# -------------------------------------------
