'''
Module with tests for the EffCal class
'''
import math
from dataclasses import dataclass

import numpy
import pytest

from rx_calibration.hltcalibration.eff_cal       import EffCal
from rx_calibration.hltcalibration.parameter     import Parameter

# --------------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    l_eff_val = numpy.linspace(0.0, 1.0, 11)
# --------------------------------
def _get_par(eff_val : float) -> Parameter:
    tot = 100

    pas         = Parameter()
    pas['nsig'] =     eff_val  * tot

    fal         = Parameter()
    fal['nsig'] = (1- eff_val) * tot

    return pas, fal
# --------------------------------
@pytest.mark.parametrize('eff', Data.l_eff_val)
def test_simple(eff : float):
    '''
    Simplest test of efficiency calculation
    '''
    pas, fal = _get_par(eff)

    obj = EffCal(pas=pas, fal=fal)
    val = obj.get_eff()

    assert math.isclose(val, eff)
