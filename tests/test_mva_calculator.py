'''
Module containing tests for MVACalculator class
'''

from ROOT                         import RDF
from dmu.logging.log_store        import LogStore
from rx_data.rdf_getter           import RDFGetter
from rx_classifier.mva_calculator import MVACalculator

log=LogStore.add_logger('rx_classifier:test_mva_calculator')
# ----------------------
def _validate_rdf(rdf : RDF.RNode) -> None:
    '''
    Parameters
    -------------
    rdf: ROOT dataframe with MVA scores
    '''
    assert isinstance(rdf, RDF.RNode)

    nentries = rdf.Count().GetValue()
    assert nentries > 0
# ----------------------
def test_simple() -> None:
    '''
    Simplest test validating MVACalculator
    '''
    sample = 'DATA_24*'
    trigger= 'Hlt2RD_BuToKpMuMu_MVA'
    version= 'v7.7'

    with RDFGetter.max_entries(value=100):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf()

    cal = MVACalculator(
    rdf     = rdf,
    sample  = sample,
    trigger = trigger,
    version = version)

    rdf = cal.get_rdf(rdf=rdf)
    _validate_rdf(rdf=rdf)
# --------------------------------
