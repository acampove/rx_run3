'''
Module with functions needed to test ConstraintReader class
'''

import pytest
from dmu.stats.zfit           import zfit
from dmu.logging.log_store    import LogStore
from fitter.constraint_reader import ConstraintReader

log=LogStore.add_logger('fitter:test_constraint_reader')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    l_kind = [
        'sig_par', 
        'rare_prec', 
        'invalid', 
        'brem_frac']
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('fitter:constraint_reader', 10)
# --------------------------------------------------------------
def _print_constraints(d_cns : dict[str, tuple[float,float]]) -> None:
    for name, (value, error) in d_cns.items():
        log.info(f'{name:<50}{value:<20.3f}{error:<20.3f}')
# --------------------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
@pytest.mark.parametrize('kind' , Data.l_kind)
def test_simple(kind : str, q2bin : str, get_parameters_holder):
    '''
    Tests getting constraints

    Parameters
    -------------
    kind : Type of parameters
    q2bin: q2 bin
    '''
    obs     = zfit.Space('dummy', limits=(4500, 6000))
    obj     = get_parameters_holder(kind=kind, obs=obs)

    obj     = ConstraintReader(obj=obj, q2bin=q2bin)
    d_cns   = obj.get_constraints()
    _print_constraints(d_cns)

    # TODO: Needs to be updated when other parameter constraints be implemented
    if kind != 'rare_prec':
        return

    assert len(d_cns) > 0 
# --------------------------------------------------------------
