'''
Module holding the Category class
'''

from omegaconf import DictConfig
from dmu.stats import zfit
from dmu       import LogStore

zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:category')
# -----------------------------------
class Category:
    '''
    Class meant to represent a fit category, i.e. a fit for a given
    dataset when splitting in e.g. blocks and/or Brem categories
    '''
    # ----------------------
    def __init__(
        self, 
        name : str, 
        model: zpdf, 
        sumw : float,
        cres : DictConfig):
        '''
        Parameters
        -------------
        name: Name of category, e.g. brem_001_b1
        model: Zfit PDF representing component after fit to MC
        sumw : Sum of weights in MC sample
        cres : Dictionary mapping variable name to value and error for parameter
        '''
        self._name = name
        self._model= model
        self._sumw = sumw
        self._cres = cres
# -----------------------------------
