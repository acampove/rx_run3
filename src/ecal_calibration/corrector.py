'''
Module containing the Corrector class
'''
from dask.dataframe import DataFrame as DDF

# ---------------------------------------------
class Corrector:
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
        self._ddf = ddf
        self._cfg = cfg
    # ---------------------------------------------
    def get_ddf(self) -> DDF:
        '''
        Returns Dask DataFrame with event kinematics
        after corrections have been applied
        '''
# ---------------------------------------------
