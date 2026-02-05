'''
Module used to test CategoryMerger class
'''

from dmu       import LogStore
from dmu.stats import zfit
from dmu.stats import utilities as sut
from omegaconf import OmegaConf
from fitter    import CategoryMerger
from fitter    import Category

zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:test_category_merger')
# ----------------------------------------
def _get_category(name : str) -> Category:
    '''
    Parameters
    -------------
    name: Nam of category

    Returns
    -------------
    Category object
    '''
    obs = zfit.Space('mass', limits = (0, 10))
    pdf = sut.get_model(
        obs    = obs,
        kind   = 'signal', 
        suffix = name)
    cres = OmegaConf.create({'mu' : [10., 1.]})

    cat  = Category(
        name      = name, 
        pdf       = pdf, 
        sumw      = 1000., 
        cres      = cres,
        selection = {'mass' : '(1)'},
        model     = ['gauss'])

    return cat
# ----------------------------------------
def test_add_brem():
    '''
    Simplest test of merger of categories
    '''
    names      = [ f'brem_00{brem}_b1' for brem in range(1, 3)  ]
    categories = [ _get_category(name = name) for name in names ]

    mgr = CategoryMerger(categories = categories)
    cat = mgr.get_category()

    assert isinstance(cat, Category)

    print(cat)
# ----------------------------------------
def test_add_block():
    '''
    Simplest test of merger of categories
    '''
    names      = [ f'brem_001_b{block}' for block in range(1, 3) ]
    categories = [ _get_category(name = name) for name in names ]

    mgr = CategoryMerger(categories = categories)
    cat = mgr.get_category()

    assert isinstance(cat, Category)

    print(cat)

