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
    def __init__(
        self,
        nll   : zlos,
        res   : zres,
        ntoys : int):
        '''
        Parameters
        -------------
        nll  : Zfit negativve log likelihood instance
        res  : Result of actual fit to data. Used to make sure
               toys are generaged with the correct initial parameters
        ntoys: Number of toys to generate
        '''
        self._nll   = nll
        self._res   = res
        self._ntoys = ntoys
    # ----------------------
    def _add_parameters(
        self, 
        df  : pnd.DataFrame, 
        res : zres, 
        gof : tuple[float,int,float],
        itoy: int) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        df: Pandas dataframe with potentially information already in it
        res: FitResult object from last fit
        gof: Tuple with pvalue, nodf and chi2 
        itoy:Index for current fit

        Returns
        -------------
        Input pandas dataframe with extra rows added
        It will add one row per parameter
        '''
        cres  = sut.zres_to_cres(res=res)
        for name, cfg_par in cres.items():
            nrows = len(df)
            df.loc[nrows] = [name, cfg_par.value, cfg_par.error, itoy, gof[0], res.converged]

        return df
