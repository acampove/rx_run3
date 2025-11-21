'''
Module with tests for FitSummary class 
'''
import pytest
from fitter   import FitSummary

# ----------------------
@pytest.mark.parametrize('force_update', [True, False])
def test_update(force_update : bool) -> None:
    '''
    Simplest test
    '''
    smr = FitSummary(name='mid_window')
    df  = smr.get_df(force_update = force_update)

    assert len(df) > 0
# ----------------------
