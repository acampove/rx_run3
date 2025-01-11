'''
Module holding tests for Parameter class
'''
import pytest

from rx_calibration.hltcalibration.parameter import Parameter

def test_simple():
    '''
    Simplest test
    '''

    obj = Parameter()
    obj['a'] = 3,3
    obj['b'] = 2,2

    assert 3,3 == obj['a']
    assert 2,2 == obj['b']

    with pytest.raises(ValueError):
        _ = obj['c']
