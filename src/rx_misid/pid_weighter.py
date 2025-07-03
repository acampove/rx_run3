'''
Module holding PIDWeighter class
'''
from ROOT                 import RDataFrame
from zfit.core.interfaces import ZfitData   as zobs
from zfit.core.interfaces import ZfitPDF    as zpdf

# ------------------------------------------------
class PIDWeighter:
    '''
    Class meant to:

    - Take ROOT dataframes from a given MC sample
    - Access PID maps
    - Calculate weights from those maps for sample
    - Use the weighted mass distribution to fit and return PDF
    '''
    # -----------------------------------------
    def __init__(self, rdf : RDataFrame):
        '''
        Parameters
        ------------------
        ROOT dataframe
        '''
        self._l_col : list[str]

        self._rdf = self._check_rdf(rdf=rdf)
    # -----------------------------------------
    def _check_rdf(self, rdf : RDataFrame) -> RDataFrame:
        '''
        Takes ROOT dataframe

        - Implements checks
        - If everything succeeds, returns dataframe
        '''

        self._l_col = [ name.c_str() for name in rdf.GetColumnNames() ]
        l_true      = [ var for var in self._l_col if '_TRUEID' in var ]
        if len(l_true) == 0:
            raise ValueError('No true columns found, this dataset is not MC')

        return rdf
    # -----------------------------------------
    def get_pdf(self, obs : zobs) -> zpdf:
        '''
        Parameters
        --------------
        obs: Zfit observable, used to fit, deduce observable, etc
        '''
        return
# ------------------------------------------------
