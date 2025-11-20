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
