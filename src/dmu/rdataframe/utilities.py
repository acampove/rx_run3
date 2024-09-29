'''
Module containing utility functions to be used with ROOT dataframes
'''

import hashlib

import numpy

from ROOT import RDataFrame, Numba

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:rdataframe:utilities')

# ---------------------------------------------------------------------
def _hash_from_numpy(arr_val):
    '''
    Will take a numpy array
    Will return the hash of a numpy array
    '''

    arr_bytes = arr_val.tobytes()
    hsh       = hashlib.sha256()
    hsh.update(arr_bytes)
    val = hsh.hexdigest()

    return val
# ---------------------------------------------------------------------
def _define_arr_getter(arr_val, hash_arr):
    '''
    Takes numpy array and corresponding hash

    Defines in Numba namespace a function to pick values from that array
    '''

    if hasattr(Numba, hash_arr):
        return

    @Numba.Declare(['int'], 'float', name=f'fun_{hash_arr}')
    def get_array_value(index):
        return arr_val[index]
# ---------------------------------------------------------------------
def add_column(rdf : RDataFrame, arr_val : numpy.ndarray | None, name : str):
    '''
    Will take a dataframe, an array of numbers and a string
    Will add the array as a colunm to the dataframe
    '''

    if not isinstance(arr_val, numpy.ndarray):
        log.error('Input array needs to be an instance of numpy.ndarray')
        raise ValueError

    v_col = rdf.GetColumnNames()
    l_col = [ col.c_str() for col in v_col ]

    nval  = len(arr_val)
    nent  = rdf.Count().GetValue()

    if nval != nent:
        log.error(f'Size of dataframe and input array differ, array/dataframe: {nval}/{nent}')
        raise ValueError

    if name in l_col:
        log.error(f'Column name {name} already found in dataframe')
        raise ValueError

    log.debug(f'Adding column {name}')

    hash_arr = _hash_from_numpy(arr_val)
    _define_arr_getter(arr_val, hash_arr)

    rdf = rdf.Define(name, f'Numba::fun_{hash_arr}(rdfentry_)')

    return rdf
# ---------------------------------------------------------------------
