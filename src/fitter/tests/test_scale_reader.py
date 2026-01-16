'''
Module with tests for ScaleReader class
'''
import pytest

from dmu       import LogStore
from fitter    import ScaleReader
from rx_common import Correction
from rx_common import Brem
from rx_common import Block 

log=LogStore.add_logger('fitter:test_scale_reader')
# ----------------------
@pytest.mark.parametrize('kind' , Correction)
@pytest.mark.parametrize('block', Block)
@pytest.mark.parametrize('brem' , [Brem.one, Brem.two])
def test_simple(
    brem : Brem,
    block: Block,
    kind : Correction):
    '''
    Simplest test
    '''
    srd      = ScaleReader()
    val, err = srd.get_scale(
        corr  = kind, 
        block = block, 
        brem  = brem)

    assert isinstance(val, float)
    assert isinstance(err, float)
