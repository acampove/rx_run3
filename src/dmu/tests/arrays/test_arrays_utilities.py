'''
Script containing unit tests for functions in arrays/utilities.py
'''
from dataclasses import dataclass

import numpy
import pytest

from dmu.arrays.utilities import repeat_arr

#------------------------------------------
@dataclass
class Data:
    '''
    Class used to store inputs for tests
    '''

    l_arg_repeat_arr = [
            (6 * [1], 1.5, 9 * [1]),
            (0 * [1], 1.5, 0 * [1]),
            ]
#------------------------------------------
@pytest.mark.parametrize('l_inp, ftimes, l_out', Data.l_arg_repeat_arr)
def test_repeat_arr(l_inp : list, ftimes : float, l_out : list):
    '''
    Tests repeat_arr function
    '''
    arr_inp = numpy.array(l_inp)
    arr_out = numpy.array(l_out)
    arr_val = repeat_arr(arr_val = arr_inp, ftimes = ftimes)

    assert numpy.equal(arr_out, arr_val).all()
#------------------------------------------
