'''
Module containing unit tests for ml/utilities.py
'''

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
def main():
    '''
    Tests start here
    '''
    test_index_with_hashes()
# ----------------------------------------
if __name__ == '__main__':
    main()
