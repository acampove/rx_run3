'''
Module used to test CategoryMerger class
'''
import pytest

from dmu       import LogStore
from dmu.stats import zfit
from dmu.stats import utilities as sut
from omegaconf import OmegaConf
from fitter    import CategoryMerger
from fitter    import Category
from rx_common import Brem

zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:test_category_merger')

BREM_012 = [ Brem.zero, Brem.one, Brem.two ]
BREM_01  = [ Brem.zero, Brem.one           ]
BREM_02  = [ Brem.zero, Brem.two           ]
BREM_12  = [ Brem.one , Brem.two           ]
# ----------------------------------------
def _get_category(name : str) -> Category:
    '''
    Parameters
    -------------
    name: Name of category

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
@pytest.mark.parametrize('brems', [BREM_012, BREM_01, BREM_02, BREM_12])
def test_add_brem(brems : list[Brem]):
    '''
    Simplest test of merger of categories
    '''
    names      = [ f'brem_{brem:03}_b1' for brem in brems ]
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
    names      = [ f'brem_xx1_b{block}' for block in range(1, 4) ]
    categories = [ _get_category(name = name) for name in names ]

    mgr = CategoryMerger(categories = categories)
    cat = mgr.get_category()

    assert isinstance(cat, Category)

    print(cat)
# ----------------------------------------
