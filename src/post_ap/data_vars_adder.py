'''
Module containing DataVarsAdder class
'''

from ROOT import RDataFrame

class DataVarsAdder:
    '''
    Class used to add variables to dataframes that only make sense for data
    '''
    # -------------------------------------
    def __init__(self, rdf : RDataFrame):
        self._rdf = rdf
    # -------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe with all variables added (or booked in this case)
        '''
        return self._rdf
# -------------------------------------
