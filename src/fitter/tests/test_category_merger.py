'''
Module used to test CategoryMerger class
'''

from dmu       import LogStore
from dmu.stats import zfit
from fitter    import CategoryMerger

zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:test_category_merger')
# ----------------------------------------
def test_simple():
    '''
    Simplest test of merger of categories
    '''
    mgr = CategoryMerger(
        pdfs       = l_pdf, 
        yields     = l_yield, 
        results    = l_cres,
        categories = categories)

    pdf = mgr.get_model()

    assert isinstance(pdf, zpdf)
# ----------------------------------------
