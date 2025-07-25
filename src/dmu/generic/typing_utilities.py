'''
This module contains utility functions related to typing
'''

from typing import TypeVar, Type
import pandas as pnd
import numpy

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:generic:typing')
Num = TypeVar('Num', int, float)
# ----------------------
def numeric_from_series(
    row     : pnd.Series,
    name    : str,
    numeric : Type[Num]) -> Num:
    '''
    This is meant to be used to silence pyright errors
    when retrieving values from pandas series

    Parameters
    --------------
    row    : Pandas dataframe row
    name   : Name of column whose value to extract
    numeric: A numeric type, supported int, float or numpy number

    Returns
    --------------
    Either an int or a float with the value of the column
    '''
    if name not in row:
        raise ValueError(f'Cannot find {name} in row: {row}')

    val = row[name]

    if not isinstance(val, (int, float, numpy.number)):
        raise ValueError(f'Value {name}={val} is not numeric')

    return numeric(val)
# ----------------------
