'''
Module used to test CmbConstraints class
'''

import zfit
import pytest 

from dmu.stats   import ConstraintND
from rx_common   import Qsq
from fitter      import CmbConstraints
from dmu         import LogStore
from dmu.generic import utilities           as gut
from dmu.stats   import ModelFactory
from zfit.loss   import ExtendedUnbinnedNLL as zlos

log=LogStore.add_logger('test_cmb_constraints')
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('fitter:cmb_constraints', 10)
# ----------------------
def _get_nll() -> zlos:
    '''
    Returns
    -------------
    Likelihood with a combinatorial PDF
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    mod = ModelFactory(
        preffix = 'combinatorial',
        obs     = obs,
        l_pdf   = ['hypexp'],
        l_shared= [],
        l_float = [])

    pdf   = mod.get_pdf()
    data  = pdf.create_sampler(1000)

    return zlos(model = pdf, data = data)
# ----------------------
@pytest.mark.parametrize('q2bin', ['low'])
def test_simple(q2bin : Qsq):
    '''
    Simplest test of CmbConstraints
    '''
    fit_cfg   = gut.load_conf(package='fitter_data', fpath = 'tests/fits/constraint_reader.yaml')
    nll       = _get_nll()

    calc      = CmbConstraints(
        nll   = nll,
        cfg   = fit_cfg,
        q2bin = q2bin)

    constraint = calc.get_constraint()

    assert isinstance(constraint, ConstraintND)

    print(constraint)
