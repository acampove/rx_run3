'''
Module with tests for utility functions
'''

from ecal_calibration   import utilities as cut

def test_get_ddf():
    '''
    Tests getter of dask dataframe
    '''
    ddf = cut.get_ddf()
    print(ddf)

