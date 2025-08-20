'''
This module contains tests for the ConstraintAdder class
'''
import pytest
from dmu.stats.constraint_adder import ConstraintAdder
from dmu.stats                  import utilities as sut
from dmu.generic                import utilities as gut

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:test_constraint_adder')
# ----------------------
@pytest.mark.parametrize('mode', ['toy', 'real'])
def test_simple(mode : str) -> None:
    '''
    This is the simplest test of ConstraintAdder

    Parameters 
    -------------
    mode : Kind of constraints that will be added
    '''
    nll = sut.get_nll(kind='s+b')
    cns = gut.load_conf(package='dmu_data', fpath='tests/stats/constraints/constraint_adder.yaml')

    cad = ConstraintAdder(nll=nll, cns=cns)
    nll = cad.get_nll(mode=mode)
