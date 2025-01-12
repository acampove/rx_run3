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

def test_to_json():
    '''
    Tests saving to JSON
    '''

    obj = Parameter()
    obj['a'] = [3,3]
    obj['b'] = [2,2]

    obj.to_json('/tmp/rx_calibration/tests/parameter/save/parameter.json')

def test_from_json():
    '''
    Tests making a parameter object from JSON
    '''
    path = '/tmp/rx_calibration/tests/parameter/save/parameter.json'

    obj_1      = Parameter()
    obj_1['b'] = [2,2]
    obj_1['a'] = [3,3]

    obj_1.to_json(path)

    obj_2=Parameter.from_json(path)

    assert obj_1 == obj_2



