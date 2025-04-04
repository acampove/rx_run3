'''
Module with functions needed to provide hashes
'''

import json
import hashlib
from typing import Any

# ------------------------------------
def hash_object(obj : Any) -> str:
    '''
    Function taking a python object and returning 
    a string representing the hash
    '''
    try:
        string = json.dumps(obj)
    except Exception as exc:
        raise ValueError(f'Cannot hash object: {obj}') from exc

    string_bin = string.encode('utf-8')
    hsh        = hashlib.sha256(string_bin)

    return hsh.hexdigest()
# ------------------------------------
