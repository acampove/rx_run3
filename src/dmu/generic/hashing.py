'''
Module with functions needed to provide hashes
'''

import json
import hashlib
from typing import Any

import pandas as pnd
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:generic.hashing')
# ------------------------------------
def _object_to_string(obj : Any) -> str:
    try:
        string = json.dumps(obj)
    except Exception as exc:
        raise ValueError(f'Cannot hash object: {obj}') from exc

    return string
# ------------------------------------
def _dataframe_to_hash(df : pnd.DataFrame) -> str:
    sr_hash = pnd.util.hash_pandas_object(df, index=True)
    values  = sr_hash.values
    hsh     = hashlib.sha256(values)
    hsh     = hsh.hexdigest()

    return hsh
# ------------------------------------
def hash_object(obj : Any) -> str:
    '''
    Function taking a python object and returning 
    a string representing the hash
    '''

    if isinstance(obj, pnd.DataFrame):
        return _dataframe_to_hash(df=obj)

    string     = _object_to_string(obj=obj)
    string_bin = string.encode('utf-8')
    hsh        = hashlib.sha256(string_bin)

    return hsh.hexdigest()
# ------------------------------------
