'''
Module will hold unit tests for functions in rdataframe/utilities.py
'''

from ROOT import RDF

import numpy
import pytest

import dmu.rdataframe.utilities as ut

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test:rdataframe:utilities')

@pytest.mark.parametrize('itry', [1, 2])
# -------------------------------------------------
def test_add_column(itry):
    '''
    Will test adding a numpy array to a ROOT dataframe
    '''
    log.info(f'This is try: {itry}')
    d_data = {
            'x' : numpy.array([1, 2, 3]),
            'y' : numpy.array([4, 5, 6]),
            }

    rdf = RDF.FromNumpy(d_data)
    rdf = rdf.Define('z', 'ROOT::RVec<int>({1, 2, 3})')
    rdf = rdf.Define('w', 'true')

    arr_val = numpy.array([10, 20, 30])

    rdf = ut.add_column(rdf, arr_val, 'values')

    rdf.Display().Print()
# -------------------------------------------------
def test_misalignment():
    '''
    Will test with inputs where columns have different sizes
    '''
    d_data = {
            'x' : numpy.array([1, 2, 3]),
            'y' : numpy.array([4, 5, 6]),
            }

    rdf     = RDF.FromNumpy(d_data)
    arr_val = numpy.array([10, 20])

    with pytest.raises(ValueError):
        rdf = ut.add_column(rdf, arr_val, 'z')

    rdf.Display().Print()
# -------------------------------------------------
def test_rdf_report_to_df():
    '''
    Will test with inputs where columns have different sizes
    '''
    d_data = {
            'x' : numpy.random.uniform(0,1,1000),
            'y' : numpy.random.uniform(0,1,1000),
            }

    rdf  = RDF.FromNumpy(d_data)
    rdf  = rdf.Filter('x > 0.2', 'x')
    rdf  = rdf.Filter('y > 0.2', 'y')

    rep  = rdf.Report()
    df   = ut.rdf_report_to_df(rep)

    print(df)
