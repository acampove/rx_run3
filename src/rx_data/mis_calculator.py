'''
Module with MisCalculator class
'''

from ROOT import RDataFrame

# ----------------------------------------------
class MisCalculator:
    '''
    Class used to add missing variables to ROOT dataframes
    '''
    # -------------
    def __init__(self, rdf : RDataFrame, trigger : str):
        '''
        Initializer taking dataframe and trigger, the latter is needed to know mass hypotheses of leptons
        '''
        self._rdf     = rdf
        self._trigger = trigger
    # -------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe after adding variables
        '''

        return self._rdf
# ----------------------------------------------
