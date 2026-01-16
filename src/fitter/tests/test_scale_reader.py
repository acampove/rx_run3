'''
Module with tests for ScaleReader class
'''

from fitter import ScaleReader

from dmu import LogStore

log=LogStore.add_logger('fitter:test_scale_reader')
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    kind  = 'reso'
    block = '1'
    brem  = '1'

    srd = ScaleReader()

    val, err = srd.get_scale(
        name  = kind, 
        block = block, 
        brem  = brem)

    assert isinstance(val, float)
    assert isinstance(err, float)
