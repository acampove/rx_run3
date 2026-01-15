'''
Module with tests for FitSummary class 
'''
import pytest
from fitter   import FitSummary

# ----------------------
@pytest.mark.parametrize('name', ['reso_non_dtf'])
def test_simple(name : str) -> None:
    '''
    Simplest test
    '''
    smr = FitSummary(name = name, signal = 'jpsi')
    df  = smr.get_df(force_update = True)

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
    smr = FitSummary(name='reso_non_dtf', signal = 'jpsi')
    df  = smr.get_df(force_update = force_update)

    assert len(df) > 0

    print('')
    print(df)
# ----------------------
