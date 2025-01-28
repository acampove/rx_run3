'''
Module containing tests for MCVarsAdder class
'''

import numpy
import pytest
from ROOT                    import RDataFrame
from ROOT                    import RDF

from post_ap.mc_vars_adder   import MCVarsAdder

# -------------------------------------------------
class Data:
    '''
    Class used to hold shared attributes
    '''
    rng = numpy.random.default_rng(seed=10)
# -------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    pass
# -------------------------------------------------
def _get_rdf(kind : str) -> RDataFrame:
    nentries = {'gen' : 1000, 'rec' : 100}[kind]

    d_data   = {}

    d_data['B_PT'] = Data.rng.uniform(0, 10_000, nentries)

    if kind == 'rec':
        d_data['EVENTNUMBER'] = Data.rng.integers(0, 1000_000, size=nentries)

    return RDF.FromNumpy(d_data)
# -------------------------------------------------
def test_add_to_gen():
    '''
    Tests addition of columns to MCDT
    '''

    rdf_gen = _get_rdf(kind='gen')
    rdf_rec = _get_rdf(kind='rec')

    obj = MCVarsAdder(rdf_gen = rdf_gen, rdf_rec=rdf_rec)
    _   = obj.get_rdf()
# -------------------------------------------------
def test_add_to_rec():
    '''
    Tests addition of columns to DecayTree
    '''

    rdf_rec = _get_rdf(kind='rec')

    obj = MCVarsAdder(rdf_rec=rdf_rec)
    _   = obj.get_rdf()
# -------------------------------------------------
