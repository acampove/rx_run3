'''
Module with tests for utility functions
'''
import matplotlib.pyplot as plt

from ecal_calibration   import utilities as cut

# -----------------------------------------
def test_get_ddf():
    '''
    Tests getter of dask dataframe
    '''
    ddf = cut.get_ddf()
    df  = ddf.compute()

    assert len(df) == 10_000
# -----------------------------------------
