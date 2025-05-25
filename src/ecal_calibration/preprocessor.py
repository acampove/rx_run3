'''
Module holding PreProcessor class
'''

from dask.dataframe.core import DataFrame as DDF

# --------------------------
class PreProcessor:
    '''
    Class used to process input data into features and target columns
    '''
    # ---------------------------------
    def __init__(self, ddf : DDF):
        '''
        ddf: Dask dataframe with raw data to preprocess
        '''
        self._ddf = ddf
# --------------------------
