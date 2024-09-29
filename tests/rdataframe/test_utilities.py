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

    arr_val = numpy.array([10, 20, 30])

    rdf = ut.add_column(rdf, arr_val, 'values')

    rdf.Display().Print()
# -------------------------------------------------
