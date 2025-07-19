'''
This script tests uses of typehinting with ROOT
'''
from ROOT import RDataFrame, RDF

def test_rdf_type() -> RDF.RNode|None:
    rdf = RDataFrame(10)
    print('testing')

    return rdf

test_rdf_type()
