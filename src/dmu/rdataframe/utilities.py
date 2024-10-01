'''
Module containing utility functions to be used with ROOT dataframes
'''

import hashlib

import numpy

from ROOT import RDataFrame, RDF, Numba

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

    if hasattr(Numba, f'fun_{hash_arr}'):
        return

    @Numba.Declare(['int'], 'float', name=f'fun_{hash_arr}')
    def get_array_value(index):
        return arr_val[index]
# ---------------------------------------------------------------------
def _arr_type_is_known(arr_val):
    '''
    Takes numpy array returns True if array stores known types
    Otherwise it returns false
    '''

    l_known_type = ['int32', 'uint32', 'int64', 'uint64', 'float32', 'float64']

    str_type = arr_val.dtype.__str__()
    is_known = str_type in l_known_type
    if not is_known:
        log.warning(f'Found unknown type: {str_type}')

    return is_known
# ---------------------------------------------------------------------
def add_column(rdf : RDataFrame, arr_val : numpy.ndarray | None, name : str, mode : bool | str = 'dict'):
    '''
    Will take a dataframe, an array of numbers and a string
    Will add the array as a colunm to the dataframe

    mode (str) : Way in which branch is created
        numba : Will create a function that returns value, fast but causes crashes
        dict  : Will dump data in memory as a dictionary and add another entry, then create a dataframe. Slow, but safer
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

    if mode == 'numba':
        hash_arr = _hash_from_numpy(arr_val)
        _define_arr_getter(arr_val, hash_arr)
        rdf = rdf.Define(name, f'Numba::fun_{hash_arr}(rdfentry_)')
    elif mode == 'dict':
        d_data_ini   = rdf.AsNumpy()
        d_data       = { key : val for key, val in d_data_ini.items() if _arr_type_is_known(val) } 
        d_data[name] = arr_val
        rdf          = RDF.FromNumpy(d_data)
    else:
        raise ValueError(f'Invalid mode: {mode}')

    return rdf
# ---------------------------------------------------------------------
