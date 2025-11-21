'''
Tests for Measurement class
'''
import pytest
from pydantic import ValidationError

from dmu import Measurement

# ------------------------------------
def test_simple():
    data = {'a' : (1., 1.), 'b' : (2., 1.)}

    ms = Measurement(data=data)
    assert ms['a'] == (1., 1.)

    with pytest.raises(ValidationError):
        ms.data = {}

    assert 'a'     in ms
    assert 'x' not in ms
# ------------------------------------
def test_print():
    '''
    Test string representations
    '''
    data = {'a' : (1., 1.), 'b' : (2., 1.)}

    ms = Measurement(data=data)

    ms.__repr__()
    ms.__str__()
# ----------------------
def test_to_dict():
    '''
    Tests to_dict method
    '''
    data = {'a' : (1., 1.), 'b' : (2., 1.)}

    ms  = Measurement(data=data)
    res = ms.to_dict()

    assert res == {
        'a' : 1., 'a_error' : 1.,
        'b' : 2., 'b_error' : 1.}
