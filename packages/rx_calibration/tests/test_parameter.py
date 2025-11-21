'''
Module holding tests for Parameter class
'''
import pytest

from dmu.logging.log_store import LogStore
from rx_calibration.hltcalibration.parameter import Parameter

log=LogStore.add_logger('rx_calibration:hltcalibration:test_parameter')
# ---------------------------------------------------------------------
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
# ---------------------------------------------------------------------
def test_to_json():
    '''
    Tests saving to JSON
    '''

    obj = Parameter()
    obj['a'] = [3,3]
    obj['b'] = [2,2]

    obj.to_json('/tmp/rx_calibration/tests/parameter/save/parameter.json')
# ---------------------------------------------------------------------
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
# ---------------------------------------------------------------------
def test_str():
    '''
    Tests __str__ dunder method
    '''
    log.info('')

    obj = Parameter()
    obj['a_x'] = 3,3
    obj['b_y'] = 2,2

    print(obj)
# ---------------------------------------------------------------------
def test_remove_suffix():
    '''
    Tests static remove_suffix method 
    '''
    log.info('')

    obj = Parameter()
    obj['a_x'] = 3,3
    obj['b_y'] = 2,2

    val_no_y = Parameter.remove_suffix(par=obj, suffix='_y')
    val_no_x = Parameter.remove_suffix(par=obj, suffix='_x')

    print(val_no_x)
    print(val_no_y)
# ---------------------------------------------------------------------
