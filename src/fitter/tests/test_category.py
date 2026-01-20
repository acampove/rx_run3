'''
Module with tests for the Category class
'''

from fitter      import Category
from omegaconf   import OmegaConf
from dmu.stats   import utilities as sut
from dmu         import LogStore

log=LogStore.add_logger('fitter:test_category')
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    pdf = sut.get_model(kind = 'signal')
    cres= OmegaConf.create(obj = {'mu' : [10., 1.]})

    cat = Category(
        name = 'category',
        model= pdf,
        sumw = 1000.,
        cres = cres)

