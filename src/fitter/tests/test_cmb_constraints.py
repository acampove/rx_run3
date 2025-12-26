'''
Module used to test CmbConstraints class
'''

import zfit
import pytest 

from dmu.stats   import ConstraintND
from rx_common   import Qsq
from fitter      import CmbConstraints
from dmu.generic import utilities as gut

# ----------------------
@pytest.mark.parametrize('q2bin', ['low'])
def test_simple(q2bin : Qsq):
    '''
    Simplest test of CmbConstraints
    '''
    fit_cfg = gut.load_conf(package='fitter_data', fpath = 'tests/fits/constraint_reader.yaml')
    obs     = zfit.Space('B_Mass', 4500, 7000)

    calc      = CmbConstraints(
        obs   = obs,
        cfg   = fit_cfg.model.components.combinatorial,
        q2bin = q2bin)

    constraint = calc.get_constraint()

    assert isinstance(constraint, ConstraintND)

