'''
Module holding CategoryMerger class
'''
import zfit

from typing      import Literal
from omegaconf   import OmegaConf
from dmu         import LogStore
from rx_common   import Brem, Block, Correction
from dmu.stats   import ModelFactory
from .category   import Category

log=LogStore.add_logger('fitter:category_merger')

zpar      = zfit.param.Parameter
Condition = Literal['all_same', 'all_different']
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
    def _enforce(
        self, 
        categories : list[Category], 
        kind       : type, 
        condition  : Condition) -> None:
        '''
        Parameters
        -------------
        categories: List of categories
        kind      : Used to enforce condition on given attribute of categories
        condition : Condition that has to be satisfied by categories
        '''
        objs : set[Brem|Block]

        if   kind is Brem:
            objs = { cat.brem  for cat in categories }
        elif kind is Block:
            objs = { cat.block for cat in categories }
        else:
            raise ValueError(f'Invalid type: {type}')

        ntotal  = len(categories)
        nunique = len(objs)

        if condition == 'all_same'      and nunique == 1: 
            return
        
        if condition == 'all_different' and ntotal  == nunique:
            return

        for obj in objs:
            log.error(obj)

        raise ValueError(f'Condition {condition} failed')
    # ----------------------
    def _merge_blocks(
        self, 
        categories : list[Category]) -> Category:
        '''
        Parameters
        -------------
        categories: List of categories, all with the same brem and different block

        Returns
        -------------
        Single category, resulting from adding all blocks
        '''
        self._enforce(
            categories = categories,
            kind       = Block, 
            condition  = 'all_different')

        self._enforce(
            categories = categories,
            kind       = Brem, 
            condition  = 'all_same')

        categories = sorted(categories)
        blocks = [ cat.block for cat in categories ]
        block  = sum(blocks[:-1], blocks[-1])
        brem   = categories[0].brem

        corr   = Correction.blok_fraction
        totalw = sum(cat.sumw for cat in categories )
        fracs  = [ self._get_frac(category = cat, totalw = totalw, corr = corr) for cat in categories ]
        pdfs   = [ cat.pdf                                                      for cat in categories ]
        pdf    = zfit.pdf.SumPDF(pdfs, fracs[:-1])
        model  = self._model_from_categories(
            single_model= True,
            categories  = categories)

        category      = Category(
            name      = f'brem_{brem}_b{block}',
            pdf       = pdf,
            sumw      = totalw,
            cres      = OmegaConf.create({}),
            model     = model, 
            selection = {'merged' : ''})

        return category
    # ----------------------
    def _group_brems(
        self, 
        categories : list[Category]) -> list[Category]:
        '''
        Parameters
        -------------
        categories: List of objects

        Returns
        -------------
        List of categories merged by Brem
        '''
        s_brem = { cat.brem for cat in categories }
        if len(s_brem) == 1:
            log.warning(f'Only one brem found, skipping merge by brem: {s_brem}')
            return categories

        cat_by_block : dict[Block,list[Category]] = dict()
        for cat in categories:
            if cat.block not in cat_by_block:
                cat_by_block[cat.block] = [cat]
            else:
                cat_by_block[cat.block].append(cat)

        self._validate_brem_in_categories(categories = cat_by_block)

        l_block_cats : list[Category] = []
        for cats_in_block in cat_by_block.values():
            category = self._merge_brems(categories = cats_in_block)
            log.debug(f'Brem merged for block {category.block}')
            l_block_cats.append(category)

        return l_block_cats
    # ----------------------
    def _merge_brems(
        self, 
        categories : list[Category]) -> Category:
        '''
        Parameters
        -------------
        categories: List of categories all in the same block, all brems are different

        Returns
        -------------
        Category resulting from merging brem categories
        '''
        self._enforce(
            categories = categories,
            kind       = Block, 
            condition  = 'all_same')

        self._enforce(
            categories = categories,
            kind       = Brem, 
            condition  = 'all_different')

        categories = sorted(categories)

        corr  = Correction.brem_fraction
        totalw= sum(cat.sumw for cat in categories)
        fracs = [ self._get_frac(category = cat, corr = corr, totalw = totalw) for cat in categories ]
        pdfs  = [ cat.pdf                                                      for cat in categories ]
        brems = [ cat.brem                                                     for cat in categories ]

        model = self._model_from_categories(
            single_model= False,        # Different brem categories can have different models
            categories  = categories)
        sumw  = totalw 
        brem  = sum(brems[:-1], brems[-1])
        pdf   = zfit.pdf.SumPDF(pdfs, fracs[:-1])

        blok  = categories[0].block
        category      = Category(
            name      = f'brem_{brem}_b{blok}',
            pdf       = pdf,
            sumw      = sumw,
            cres      = OmegaConf.create({}),
            model     = model, 
            selection = {'merged' : ''})

        return category
    # ----------------------
    def _model_from_categories(
        self, 
        single_model : bool,
        categories   : list[Category]) -> list[str]:
        '''
        Parameters
        -------------
        single_model : If true, it will raise if categories contain different models
        categories   : List of categories

        Returns
        -------------
        List of strings, when each represents a model, e.g. [cbl, cbr]
        '''
        models : list[str] = []

        first = categories[0].model
        for cat in categories:
            if cat.model != first and single_model:
                raise ValueError(f'Found categories with different models: {cat.model} != {first}')

            models += cat.model

        return models 
    # ----------------------
    def _get_frac(
        self, 
        category : Category,
        totalw   : float,
        corr     : Correction) -> zpar:
        '''
        Parameters
        -------------
        category : Object representing fitting category
        totalw   : Full yield across all categories that this fraction is used for
        corr     : Type of correction for which this fraction is needed

        Returns
        -------------
        Fraction used to form model
        '''
        suffix = f'{corr.nickname}_{category.brem}_b{category.block}'
        value  = category.sumw / totalw

        with ModelFactory.reparametrization_parameters(floating = True):
            frac         = ModelFactory.get_reparametrization(
                kind     = corr.kind,
                par_name = f'fr_{suffix}',
                value    = value,
                low      = 0.0,
                high     = 1.0)

        return frac 
    # ----------------------
    def _validate_brem_in_categories(
        self, 
        categories : dict[Block, list[Category]]) -> None:
        '''
        Parameters
        ----------------
        categories: dictionary where values are lists of categories, all in same block 

        This method makes sure that every block contains the same
        brem categories
        '''
        s_brem : set[Brem] | None = None
        fail = False
        for cat_in_block in categories.values():
            current_brems = { cat.brem for cat in cat_in_block }
            if s_brem is None:
                s_brem = current_brems 
                continue

            if s_brem != current_brems:
                log.error(current_brems)
                fail = True

        if fail:
            raise ValueError(f'Inconsistent brem categories found: {s_brem}')
    # ----------------------
    def get_category(self) -> Category:
        '''
        Returns
        -------------
        Category object resulting from merging input categories
        '''
        cat_1 = self._group_brems(categories  = self._categories) 
        cat_2 = self._merge_blocks(categories = cat_1)

        return cat_2
# ----------------------
