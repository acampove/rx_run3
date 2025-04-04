'''
Module with functions needed to test functions in generic/hashing.py module
'''

import pandas as pnd

from dmu.generic           import hashing
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test_hashing')
# --------------------------------------
def test_hash_simple():
    '''
    Function testing the hashing of a s list of python objects
    '''
    obj = [1, 'name', [1, 'sub', 'list'], {'x' : 1}]
    val = hashing.hash_object(obj)

    log.info(f'Hash: {val}')

    assert len(val) == 64
# --------------------------------------
def test_hash_pandas():
    '''
    Function testing the hashing of a pandas dataframe 
    '''

    data = {
        'Name': ['Alice', 'Bob', 'Charlie'],
        'Age' : [25, 30, 35],
        'City': ['New York', 'London', 'Paris']
        }

    df = pnd.DataFrame(data)

    val = hashing.hash_object(obj=df)

    log.info(f'Hash: {val}')

    assert len(val) == 64
# --------------------------------------
