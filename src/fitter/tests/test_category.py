'''
Module with tests for the Category class
'''
import pytest

from fitter      import Category
from omegaconf   import OmegaConf
from dmu.stats   import utilities as sut
from dmu         import LogStore
from rx_common   import Block, Brem

log=LogStore.add_logger('fitter:test_category')
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    pdf = sut.get_model(kind = 'signal')
    cres= OmegaConf.create(obj = {'mu' : [10., 1.]})

    cat = Category(
        name      = 'category',
        pdf       = pdf,
        sumw      = 1000.,
        cres      = cres,
        model     = ['gauss'],
        selection = {'mass' : '(1)'})

    assert cat.pdf == pdf

    print(cat)
# ----------------------
@pytest.mark.parametrize('brem' , Brem)
@pytest.mark.parametrize('block', Block)
def test_properties(brem : Brem, block : Block):
    '''
    Tests that properties are calculated correctly
    '''
    pdf = sut.get_model(kind = 'signal')
    cres= OmegaConf.create(obj = {'mu' : [10., 1.]})

    cat = Category(
        name      = f'brem_{brem}_b{block}',
        pdf       = pdf,
        sumw      = 1000.,
        cres      = cres,
        model     = ['gauss'],
        selection = {'mass' : '(1)'})

    assert cat.brem == brem
    assert cat.block== block 
