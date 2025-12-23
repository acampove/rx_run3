'''
Module with functions needed to handle random numbers
'''
import zfit
import numpy
import random 

from contextlib import contextmanager

# ----------------------
@contextmanager
def seed(value : int):
    '''
    Context manager used to set random seed

    Parameters
    -------------
    value: Random seed value, e.g. 42
    '''
    zfit.settings.set_seed(value)
    numpy.random.seed(value)
    random.seed(value)

    try:
        yield
    finally:
        zfit.settings.set_seed()
        numpy.random.seed()
        random.seed()
