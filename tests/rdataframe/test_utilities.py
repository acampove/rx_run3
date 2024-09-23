'''
Module will hold unit tests for functions in rdataframe/utilities.py
'''

from ROOT import RDF

import numpy

import dmu.rdataframe.utilities as ut

# -------------------------------------------------
def test_add_column():
    '''
    Will test adding a numpy array to a ROOT dataframe 
    '''
    d_data = {
            'x' : numpy.array([1, 2, 3]),
            'y' : numpy.array([4, 5, 6]),
            }

    rdf = RDF.FromNumpy(d_data)

    arr_val = numpy.array([10, 20, 30])

    rdf = ut.add_column(rdf, arr_val, 'values')

    rdf.Display().Print()
# -------------------------------------------------
def main():
    '''
    Tests start here
    '''
    test_add_column()
# -------------------------------------------------
if __name__ == '__main__':
    main()
