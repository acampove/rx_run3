'''
Module used to test CmbConstraints class
'''

from dmu.stats   import ConstraintND
from fitter      import CmbConstraints
from dmu.generic import utilities as gut

# ----------------------
def test_simple():
    '''
    Simplest test of CmbConstraints
    '''
    cfg = gut.load_conf(package='fitter_data', fpath = 'tests/fits/constraint_reader.yaml')

    calc      = CmbConstraints(
        obs   = cfg.observable,
        cfg   = cfg.fit_cfg.model.components.combinatorial,
        q2bin = cfg.q2bin)

    constraints = calc.get_constraints()

    assert isinstance(constraints, list)
    assert all( isinstance(const, ConstraintND) for const in constraints )

