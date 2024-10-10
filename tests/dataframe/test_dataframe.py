'''
Tests for functions in dmu/polars/interface.py
i.e. utilities to ease interfacing with polars
'''

from dmu.dataframe.dataframe import DataFrame

# --------------------------------------
def _get_df():
    '''
    Used to get polars dataframe for tests
    '''
    df = DataFrame({
    'a': [1, 2, 3],
    'b': [4, 5, 6]
    })

    return df
# --------------------------------------
def test_define():
    '''
    Tests define method
    '''
    df = _get_df()

    df = df.define('c', 'a + b')
    df = df.define('d', '((a > 1) & (b < 6)) * 0.1')

    print(df)
