'''
Module containing the Regressor class
'''
from dask.dataframe import DataFrame as DDF

# ---------------------------------------------
class Regressor:
    '''
    Class used to train a regressor to _learn_ energy
    corrections
    '''
    # ---------------------------------------------
    def __init__(self, ddf : DDF, cfg : dict):
        '''
        Parameters
        -------------------
        ddf : Dask DataFrame holding the data needed for the regression.
        cfg : Dictionary holding configuration
        '''
    # ---------------------------------------------
    def train(self) -> None:
        '''
        Will train the regressor
        '''
    # ---------------------------------------------
    def test(self) -> None:
        '''
        Will test performance of regressor
        '''
# ---------------------------------------------
