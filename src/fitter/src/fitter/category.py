'''
Module holding the Category class
'''

import yaml

from dataclasses import dataclass
from omegaconf   import DictConfig
from dmu         import LogStore
from dmu.stats   import zfit
from dmu.stats   import utilities as sut

zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:category')
# -----------------------------------
@dataclass
class Category:
    '''
    Class meant to represent a fit category, i.e. a fit for a given
    dataset when splitting in e.g. blocks and/or Brem categories

    Attributes

    name     : Name of component, e.g. brem_001_b1
    pdf      : Zfit PDF associated to category
    sumw     : Yield in MC sample associated to category
    cres     : Dictionary mapping name of variable to value and error
    model    : List of model names used for category
    selection: Selection that was used to get category, on top of nominal
    '''
    name      : str 
    pdf       : zpdf 
    sumw      : float
    cres      : DictConfig
    model     : list[str]
    selection : dict[str,str]
    # ----------------------------
    def __str__(self) -> str:
        pdfs  = sut.print_pdf(pdf = self.pdf, level = 10)

        value = f'Name: {self.name}\n'
        value+= f'Sumw: {self.sumw:.2f}\n'
        value+= f'Models: {self.model}\n'
        value+= 'Selection:\n'
        value+= yaml.safe_dump(self.selection) + '\n'
        value+= 'PDF:\n'
        value+= '\n'.join(pdfs)

        return value
# -----------------------------------
