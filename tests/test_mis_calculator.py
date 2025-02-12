'''
Module with functions testing MisCalculator
'''

from ROOT                   import RDataFrame
from rx_data.mis_calculator import MisCalculator

# ------------------------
def _get_rdf() -> RDataFrame:

# ------------------------
def test_simple():
    '''
    Simplest test of class
    '''
    rdf = _get_rdf()
    cal = MisCalculator(rdf=rdf)
    rdf = get_rdf()

    _check_rdf(rdf)
