'''
Module containing generic utility functions
'''

import time
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
