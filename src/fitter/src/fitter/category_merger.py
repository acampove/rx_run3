'''
Module holding CategoryMerger class
'''
import zfit

from omegaconf   import OmegaConf
from dmu         import LogStore
from rx_common   import Brem, Block, Correction
from dmu.stats   import ModelFactory
from .category   import Category

log=LogStore.add_logger('fitter:category_merger')

zpar = zfit.param.Parameter
# ----------------------
class CategoryMerger:
    '''
    Class meant to merge PDFs, where these PDFs:

    - Come from fits to simulated samples
    - Each sample corresponds to a given brem category and block
    '''
    # ----------------------
    def __init__(self, categories : list[Category]):
        '''
        Parameters
        -------------
        categories: List of Category objects, representing fits to MC datasets 
        '''
        self._categories = categories
    # ----------------------
    def _merge_brem_categories(self) -> list[Category]:
        '''
        Returns
        -------------
        List of categories, one per block
        '''
        cat_by_block : dict[Block,list[Category]] = dict()

        s_brem = { cat.brem for cat in self._categories }
        if len(s_brem) == 1:
            log.warning(f'Not merging by brem, found only: {s_brem}')
            return self._categories

        for cat in self._categories:
            if cat.block in cat_by_block:
                cat_by_block[cat.block].append(cat)
            else:
                cat_by_block[cat.block] = [cat]

        for block, categories in cat_by_block.items():
            if len(categories) != 2:
                raise ValueError(f'Not found two categories for block: {block}')

        return [ c1 + c2 for [c1,c2] in cat_by_block.values() ]
    # ----------------------
    def get_category(self) -> Category:
        '''
        Returns
        -------------
        Category object resulting from merging input categories
        '''

# ----------------------
