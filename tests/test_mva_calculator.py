'''
Module containing tests for MVACalculator class
'''

import pytest
from ROOT                         import RDF, RDataFrame
from dmu.logging.log_store        import LogStore
from rx_data.rdf_getter           import RDFGetter
from rx_classifier.mva_calculator import MVACalculator

log=LogStore.add_logger('rx_classifier:test_mva_calculator')

# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    l_trigger= ['Hlt2RD_BuToKpMuMu_MVA', 'Hlt2RD_BuToKpEE_MVA']
# ----------------------
def _validate_rdf(rdf : RDF.RNode) -> None:
    '''
    Parameters
    -------------
    rdf: ROOT dataframe with MVA scores
    '''
    assert isinstance(rdf, (RDF.RNode, RDataFrame))

    nentries = rdf.Count().GetValue()
    assert nentries > 0
# ----------------------
@pytest.mark.parametrize('trigger', Data.l_trigger)
def test_simple(trigger : str) -> None:
    '''
    Simplest test validating MVACalculator
    '''
    sample = 'DATA_24*'
    version= 'v7p7'

    with RDFGetter.max_entries(value=100):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf()

    cal = MVACalculator(
    rdf     = rdf,
    sample  = sample,
    trigger = trigger,
    version = version)

    rdf = cal.get_rdf()
    _validate_rdf(rdf=rdf)
# --------------------------------
