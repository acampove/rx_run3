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
    def __init__(self, categories : dict[str,Category]):
        '''
        Parameters
        -------------
        categories: Dictionary 

        Returns
        -------------
        
        '''
        
