'''
Module containing unit tests for ml/utilities.py
'''

import numpy
import pandas as pnd

import dmu.ml.utilities as ut

# ----------------------------------------
def test_index_with_hashes():
    '''
    This tests `index_with_hashes`
    '''
    d_data = {'x' : [1,2,3], 'y' : [4,5,6]}
    df     = pnd.DataFrame(d_data)

    df     = ut.index_with_hashes(df)

    print(df)
# ----------------------------------------
def test_cleanup():
    '''
    Function used to test cleanup of dataframes
    '''
    d_data_in = {
            'a' : [1., 2.,  numpy.nan, 4.],
            'b' : [5., 6.,         7., 8.],
            }

    d_data_ou = {
            'a' : [1., 2.,            4.],
            'b' : [5., 6.,            8.],
            }

    df_in = pnd.DataFrame(d_data_in)

    df_ts = ut.cleanup(df_in)
    df_ts = df_ts.reset_index(drop=True)

    df_ou = pnd.DataFrame(d_data_ou)

    assert df_ou.equals(df_ts)
# ----------------------------------------
def test_patch_and_tag():
    '''
    Will test patch_and_tag
    '''
    patch = 123.

    d_data_in = {
            'a' : [1., 2.,  numpy.nan, 4.],
            'b' : [5., 6.,         7., 8.],
            }

    d_data_ot = {
            'a' : [1., 2.,      patch, 4.],
            'b' : [5., 6.,         7., 8.],
            }

    df_in = pnd.DataFrame(d_data_in)
    df_ot = pnd.DataFrame(d_data_ot)
    df_pa = ut.patch_and_tag(df_in, value=patch)

    arr_index = df_pa.attrs['patched_indices']

    assert arr_index.tolist() == [2]
    assert df_pa.equals(df_ot)
