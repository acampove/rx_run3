'''
This script tests uses of typehinting with ROOT
'''
from ROOT import RDataFrame

def test_rdf_type() -> RDataFrame|None:
    rdf = RDataFrame(10)

    return rdf
