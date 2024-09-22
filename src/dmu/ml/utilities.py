'''
Module containing utility functions for ML tools 
'''

import hashlib

import pandas as pnd

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:utilities')

# ----------------------------------
def get_hashes(df_ft):
    '''
    Will return hashes for each row in the feature dataframe
    '''
    if not isinstance(df_ft, pnd.DataFrame):
        log.error('Features need to be in a pandas dataframe')
        raise ValueError

    s_hash = { hash_from_row(row) for _, row in df_ft.iterrows() }

    return s_hash
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

    hsh_val = hsh.digest()

    return hsh_val
# ----------------------------------
