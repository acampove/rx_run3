'''
This module contains tests for the functions in the naming.py module
'''
import pytest

from dmu.generic import naming

# --------------------------------------
@pytest.mark.parametrize('name, clean', [
                         ('a  b', 'a_b'),
                         ('a$b' , 'a_b'),
                         ('a > b' ,'a_gt_b'),
                         ('a < b' ,'a_lt_b'),
                         ('a = b' ,'a_eq_b'),
                         ('{a}'   ,'_a_'),
                         ('a.b'   ,'apb'),
                         ('a && b','a_and_b'),
                         ('a || b','a_or_b')])
def test_clean_special_characters(name : str, clean : str):
    '''
    Test of clean_special_characters function
    '''
    value = naming.clean_special_characters(name=name)
    assert value == clean
# --------------------------------------
