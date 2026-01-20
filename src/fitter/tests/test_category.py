'''
Module with tests for the Category class
'''
import pytest

from fitter      import Category
from omegaconf   import OmegaConf
from dmu.stats   import zfit
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
# ----------------------
def test_brem_sum():
    '''
    Test adding brem categories in same block
    '''
    cres= OmegaConf.create(obj = {'mu' : [10., 1.]})

    obs     = zfit.Space('mass', limits=(4500, 7000))
    pdf_001 = sut.get_model(kind = 'signal', suffix = '001', obs = obs)
    cat_001 = Category(
        name      = 'brem_001_b1',
        pdf       = pdf_001,
        sumw      = 1000.,
        cres      = cres,
        model     = ['gauss'],
        selection = {'mass' : '(1)'})

    pdf_002 = sut.get_model(kind = 'signal', suffix = '002', obs = obs)
    cat_002 = Category(
        name      = 'brem_002_b1',
        pdf       = pdf_002,
        sumw      = 1000.,
        cres      = cres,
        model     = ['gauss'],
        selection = {'mass' : '(1)'})

    cat = cat_001 + cat_002

    assert isinstance(cat, Category)

    print(cat)
