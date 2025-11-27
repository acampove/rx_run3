'''
Module with functions needed to test ConstraintReader class
'''

import pytest
from dmu.logging.log_store    import LogStore
from fitter.constraint_reader import ConstraintReader

from zfit.param               import Parameter as zpar

log=LogStore.add_logger('fitter:test_constraint_reader')

# ----------------------
class Parameters:
    '''
    Class meant to be used for test
    '''
    def get_params(self, floating : bool) -> set[zpar]:
        '''
        Returns set of zfit parameter instances
        '''
        _ = floating

        a = zpar('a', 0, 0, 1)
        b = zpar('b', 0, 0, 1)

        return {a, b}
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
def test_simple(kind : str, q2bin : str):
    '''
    Tests getting constraints

    Parameters
    -------------
    kind : Type of parameters
    q2bin: q2 bin
    '''
    obj     = Parameters()
    obj     = ConstraintReader(obj=obj, q2bin=q2bin)
    d_cns   = obj.get_constraints()
    _print_constraints(d_cns)

    # TODO: Needs to be updated when other parameter constraints be implemented
    if kind != 'rare_prec':
        return

    assert len(d_cns) > 0 
# --------------------------------------------------------------
