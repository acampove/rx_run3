'''
Module containing the KinematicsVarsAdder class
'''

from ROOT import RDataFrame

# ------------------------------------------------------------------
class KinematicsVarsAdder:
    '''
    Class that adds kinematic variables to RDataFrame
    '''
    def __init__(self, rdf : RDataFrame, variables : list[str]):
        self._rdf   = rdf
        self._l_var = variables
    # --------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Will return dataframe with variables added
        '''
        return self._rdf
# ------------------------------------------------------------------
