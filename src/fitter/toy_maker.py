'''
Module holding ToyMaker class
'''

from zfit.interface        import ZfitLoss
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:zfit_maker')
# ----------------------
class ToyMaker:
    '''
    This class is meant to:
    
    - Take NLL objects build with zfit
    - Recreate them with toy data
    - Minimize them
    - Collect the results of the fits in a pandas dataframe and return it
    '''
    # ----------------------
    def __init__(self, nll : ZfitLoss):
        '''
        Parameters
        -------------
        nll: Zfit negativve log likelihood instance
        '''
        self._nll = nll

