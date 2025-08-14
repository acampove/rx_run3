'''
Module holding ToyMaker class
'''
import tqdm
import pandas as pnd

from zfit.interface             import ZfitLoss   as zlos
from zfit.minimizers.interface  import ZfitResult as zres
from dmu.stats                  import utilities  as sut
from dmu.logging.log_store      import LogStore
from dmu.stats.fitter           import Fitter

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

