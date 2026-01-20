'''
Module holding CategoryMerger class
'''
from dmu       import LogStore
from .category import Category

log=LogStore.add_logger('fitter:category_merger')
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
    def get_category(self) -> Category:
        '''
        Returns
        -------------
        Category object resulting from merging input categories
        '''

# ----------------------
