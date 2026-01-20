'''
Module holding the Category class
'''

from dataclasses import dataclass
from omegaconf   import DictConfig
from dmu.stats   import zfit
from dmu         import LogStore

zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:category')
# -----------------------------------
@dataclass
class Category:
    '''
    Class meant to represent a fit category, i.e. a fit for a given
    dataset when splitting in e.g. blocks and/or Brem categories
    '''
    name      : str 
    pdf       : zpdf 
    sumw      : float
    cres      : DictConfig
    selection : dict[str,str]
    model     : list[str]
# -----------------------------------
