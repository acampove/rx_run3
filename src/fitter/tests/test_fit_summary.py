'''
Module with tests for FitSummary class 
'''
import pytest
from fitter   import FitSummary

# ----------------------
@pytest.mark.parametrize('kind', ['reso_no_dtf'])
def test_simple(kind : str) -> None:
    '''
    Simplest test
    '''
    smr = FitSummary(name = kind)
    df  = smr.get_df(force_update = False)

    assert len(df) > 0

    print('')
    print(df)
    for column in df.columns:
        print(column)
# ----------------------
@pytest.mark.parametrize('force_update', [True, False])
def test_update(force_update : bool) -> None:
    '''
    Test updating the parquet file
    '''
    smr = FitSummary(name='mid_window')
    df  = smr.get_df(force_update = force_update)

    assert len(df) > 0

    print('')
    print(df)
# ----------------------
