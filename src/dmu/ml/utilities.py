'''
Module containing utility functions for ML tools
'''

import hashlib
from typing import Union

import pandas as pnd

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:utilities')

# ----------------------------------
def get_hashes(df_ft : pnd.DataFrame, rvalue : str ='set') -> Union[set, list]:
    '''
    Will return hashes for each row in the feature dataframe

    rvalue (str): Return value, can be a set or a list
    '''

    if   rvalue == 'set':
        res = { hash_from_row(row) for _, row in df_ft.iterrows() }
    elif rvalue == 'list':
        res = [ hash_from_row(row) for _, row in df_ft.iterrows() ]
    else:
        log.error(f'Invalid return value: {rvalue}')
        raise ValueError

    return res
# ----------------------------------
def hash_from_row(row):
    '''
    Will return a hash from a pandas dataframe row
    corresponding to an event
    '''
    l_val   = [ str(val) for val in row ]
    row_str = ','.join(l_val)
    row_str = row_str.encode('utf-8')

    hsh = hashlib.sha256()
    hsh.update(row_str)

    hsh_val = hsh.hexdigest()

    return hsh_val
# ----------------------------------
def index_with_hashes(df):
    '''
    Will:
    - take dataframe with features
    - calculate hashes and add them as the index column
    - drop old index column
    '''

    l_hash = get_hashes(df, rvalue='list')
    ind_hsh= pnd.Index(l_hash)

    df = df.set_index(ind_hsh, drop=True)

    return df
# ----------------------------------
