'''
Module with tests for the EffCal class
'''
import math
from dataclasses import dataclass

import numpy
import pytest

from rx_calibration.hltcalibration               import test_utilities as tut
from rx_calibration.hltcalibration.eff_cal       import EffCal
from rx_calibration.hltcalibration.parameter     import Parameter

# --------------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    l_eff_val = numpy.linspace(0.2, 0.8, 7)
# --------------------------------
def _get_par(eff_val : float) -> Parameter:
    tot = 100

    pas         = Parameter()
    pas['nsign']=     eff_val  * tot

    fal         = Parameter()
    fal['nsign']= (1- eff_val) * tot

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
# --------------------------------
@pytest.mark.parametrize('eff', Data.l_eff_val)
def test_integration(eff : float):
    '''
    Tests full chain
    '''
    eff_str  = f'{eff:.3f}'
    eff_str  = eff_str.replace('.', 'p')

    name     = f'effcal/integration_{eff_str}'
    pas, fal = tut.get_fit_parameters(eff = eff, name = name)

    name     = f'{tut.Data.out_dir}/effcal/integration_{eff_str}'
    pas.to_json(f'{name}_pas.json')
    fal.to_json(f'{name}_fal.json')

    obj = EffCal(pas=pas, fal=fal)
    val = obj.get_eff()

    assert math.isclose(eff, val, rel_tol=0.01)
