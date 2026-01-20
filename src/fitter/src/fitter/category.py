'''
Module holding the Category class
'''

import re
import yaml

from typing      import Final, Self
from functools   import cached_property
from dataclasses import dataclass
from omegaconf   import DictConfig, OmegaConf
from dmu         import LogStore
from dmu.stats   import ModelFactory, zfit
from dmu.stats   import utilities as sut
from rx_common   import Block, Brem, Correction

zpar = zfit.param.Parameter
zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:category')

CATEGORY_REGEX : Final[str] = r'brem_(\d{3})_b(\d)'
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
    # ----------------------
    @cached_property
    def block(self) -> Block:
        '''
        Returns
        -------------
        Block associated to this category
        '''
        mtch = re.match(CATEGORY_REGEX, self.name)
        if not mtch:
            raise ValueError(f'Cannot extract block information from {self.name}')

        block_str = mtch.group(2)

        return Block(block_str)
    # ----------------------
    @cached_property
    def brem(self) -> Brem:
        '''
        Returns
        -------------
        Block associated to this category
        '''
        mtch = re.match(CATEGORY_REGEX, self.name)
        if not mtch:
            raise ValueError(f'Cannot extract brem information from {self.name}')

        brem_str = mtch.group(1)

        return Brem(brem_str)
    # ----------------------
    def __add__(self, other : Self) -> Self:
        '''
        Parameters
        -------------
        other: Category object that needs to be added to this one

        Returns
        -------------
        Result of sum of categories
        '''
        if self.block == other.block:
            return self._add_brem(other = other)

        raise ValueError(f'Cannot add current category to: {other}')
    # ----------------------
    def _add_brem(self, other : Self) -> Self:
        '''
        Parameters
        -------------
        other: Brem category to merge to this one, blocks are meant to be equal

        Returns
        -------------
        Merged category
        '''
        if self.brem == other.brem:
            raise ValueError(f'Cannot merge by brem, categories with same brem: {self.brem}')

        if self.model != other.model:
            raise ValueError(f'Models for brem categories are different: {self.model} != {other.model}')

        frac = self._get_frac(
            corr   = Correction.brem_fraction, 
            prefix = 'fr')

        pdf = zfit.pdf.SumPDF([self.pdf, other.pdf], frac)

        cres = OmegaConf.merge(self.cres, other.cres)
        if not isinstance(cres, DictConfig):
            raise ValueError(f'Config result object is not a DictConfig after merge: {cres}')

        return type(self)(
            name      = '',
            pdf       = pdf,
            sumw      = self.sumw + other.sumw,
            cres      = cres,
            model     = self.model,
            selection = {'merged' : ''}) # Cannot merge selections for categories with different selections
    # ----------------------
    def _get_frac(
        self, 
        corr   : Correction, 
        prefix : str) -> zpar:
        '''
        Parameters
        -------------
        corr  : Type of correction for which this fraction is needed
        prefix: Used to form name of parameter

        Returns
        -------------
        Fraction used to form model
        '''
        if corr == Correction.brem_fraction:
            suffix = f'brem_{self.brem}_b{self.block}'
        else:
            raise ValueError(f'Invalid kind: {corr}')

        # Brem/block resolution needs to be fixed to 1 when building model
        # It has to be let floating when fitting to data
        with ModelFactory.reparametrization_parameters(floating = False):
            frac         = ModelFactory.get_reparametrization(
                kind     = corr.kind,
                par_name = f'{prefix}_{suffix}',
                value    = 0.5,
                low      = 0.0,
                high     = 1.0)

        return frac 
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
