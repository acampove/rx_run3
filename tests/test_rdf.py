'''
Test for ROOT dataframe typing
'''
from ROOT import RDF
from ROOT import RDataFrame

# ----------------------
def _get_rdf() -> RDF.RNode:
    '''
    Should not return anything
    '''
    rdf = RDataFrame()
    return rdf
# ----------------------
def test_return_rdf() -> None:
    '''
    Tests that returning None instead of RDF.RNode shows an error
    '''
    _get_rdf()
# ----------------------
def test_experimental() -> None:
    RDF.Experimental.FromSpec('file.json')
# ----------------------
def test_asnumpy() -> None:
    rdf = RDataFrame()
    rdf.AsNumpy()
# ----------------------
def test_count() -> None:
    rdf = RDataFrame()
    ptr = rdf.Count()
    ptr.GetValue()
# ----------------------
def test_columns() -> None:
    rdf = RDataFrame()
    rdf.GetColumnNames()
# ----------------------
def test_filter() -> None:
    rdf = RDataFrame()
    rdf.Filter('x', 'y')
    rdf.Filter('x')
# ----------------------
def test_range() -> None:
    rdf = RDataFrame()
    rdf.Range(begin=2, end=3)
    rdf.Range(end=3)
# ----------------------
def test_report() -> None:
    rdf = RDataFrame()
    rdf.Report()
# ----------------------
def test_snapshot() -> None:
    rdf = RDataFrame()
    rdf.Snapshot('tree', 'file.root')


