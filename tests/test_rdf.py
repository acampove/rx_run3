'''
Test for ROOT dataframe typing
'''

from ROOT import RDF
from ROOT import RDataFrame

# ----------------------
def _get_rdf(val : bool) -> RDF.RNode:
    '''
    Should not return anything
    '''
    if val:
        return

    rdf = RDataFrame()
    return rdf
# ----------------------
def test_return() -> None:
    '''
    Tests that returning None instead of RDF.RNode shows an error
    '''
    _get_rdf(val =  True)
    _get_rdf(val = False)

