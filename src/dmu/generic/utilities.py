'''
Module containing generic utility functions
'''
import os
import time
import json
import inspect

from typing import Callable

from functools             import wraps
from dmu.logging.log_store import LogStore

TIMER_ON=False

log = LogStore.add_logger('dmu:generic:utilities')

# --------------------------------
def _get_module_name( fun : Callable) -> str:
    mod = inspect.getmodule(fun)
    if mod is None:
        raise ValueError(f'Cannot determine module name for function: {fun}')

    return mod.__name__
# --------------------------------
def timeit(f):
    '''
    Decorator used to time functions, it is turned off by default, can be turned on with:

    from dmu.generic.utilities import TIMER_ON
    from dmu.generic.utilities import timeit 

    TIMER_ON=True

    @timeit
    def fun():
        ...
    '''
    @wraps(f)
    def wrap(*args, **kw):
        if not TIMER_ON:
            result = f(*args, **kw)
            return result

        ts = time.time()
        result = f(*args, **kw)
        te = time.time()
        mod_nam = _get_module_name(f)
        fun_nam = f.__name__
        log.info(f'{mod_nam}.py:{fun_nam}; Time: {te-ts:.3f}s')

        return result
    return wrap
# --------------------------------
def dump_json(data, path : str, sort_keys : bool = False):
    '''
    Saves data as JSON

    Parameters
    data     : dictionary, list, etc
    path     : Path to JSON file where to save it
    sort_keys: Will set sort_keys argument of json.dump function 
    '''
    dir_name = os.path.dirname(path)
    os.makedirs(dir_name, exist_ok=True)

    with open(path, 'w', encoding='utf-8') as ofile:
        json.dump(data, ofile, indent=4, sort_keys=sort_keys)
# --------------------------------
