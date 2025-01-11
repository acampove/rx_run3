'''
Module with tests for the EffCal class
'''
from dataclasses import dataclass

import numpy
import pytest

from rx_calibration.hltcalibration.eff_cal   import EffCal 
from rx_calibration.hltcalibration.parameter import Parameter

# --------------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    l_eff_val = numpy.linspace(0.0, 1.0, 11)
# --------------------------------
def _get_par(eff_val : float) -> Parameter:
    pas = Parameter()
    fal = Parameter()

    return pas, fal
# --------------------------------
@pytest.mark.parametrize('eff_val', Data.l_eff_val)
def test_simple(eff_val : float):
    '''
    Simplest test of efficiency calculation
    '''
    pas, fal = _get_par(eff_val)

    obj = EffCal(pas=pas, fal=fal)
    eff = obj.get_eff()

    #assert eff == eff_val
