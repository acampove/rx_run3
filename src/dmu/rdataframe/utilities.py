'''
Module containing utility functions to be used with ROOT dataframes
'''

import hashlib
from dataclasses import dataclass

import numpy

from ROOT import RDataFrame, RDF, Numba

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:rdataframe:utilities')

# ---------------------------------------------------------------------
@dataclass
class Data:
    '''
    Class meant to store data that is shared
    '''
    l_good_type = [int, numpy.bool_, numpy.int32, numpy.uint32, numpy.int64, numpy.uint64, numpy.float32, numpy.float64]
    d_cast_type = {'bool': numpy.int32}
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
def _arr_type_is_known(name, arr_val):
    '''
    Takes 
    name of column
    numpy array  with numerical values

    returns True if array stores known types
    Otherwise it returns false
    '''

    if len(arr_val) == 0:
        return True

    elm = arr_val[0]

    # If this is a vector, drop it.
    elm_type = type(elm)
    str_type = str(elm_type) 
    if str_type.startswith('RVec<'):
        return False

    # If the array has a bool, it contains bools and therefore is a known type
    if (elm is True) or (elm is False):
        return True 

    is_known = elm_type in Data.l_good_type
    if not is_known:
        log.warning(f'Found unknown type {str_type} in column {name}')

    return is_known
# ---------------------------------------------------------------------
def _cast_to_valid_type(arr_val):
    '''
    Takes array of known types
    Returns array of acceptable (when saving data to RDataframe) types, e.g. bool -> int
    '''
    if len(arr_val) == 0:
        return arr_val 

    # If this is an array of bools, it will contain a bool.
    # Cast as smallest int possible and return
    elm = arr_val[0]
    if elm in [True, False]:
        return arr_val.astype(numpy.int32)

    src_type = type(elm)
    str_type = str(src_type) 
    if src_type not in Data.l_good_type:
        raise ValueError(f'Type {str_type} not valid')

    if str_type not in Data.d_cast_type:
        return arr_val

    trg_type = Data.d_cast_type[str_type]

    return arr_val.astype(trg_type)
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
        d_data_knw   = { key : val                      for key, val in d_data_ini.items() if _arr_type_is_known(key, val) } 
        d_data       = { key : _cast_to_valid_type(val) for key, val in d_data_knw.items() } 
        d_data[name] = arr_val
        rdf          = RDF.FromNumpy(d_data)
    else:
        raise ValueError(f'Invalid mode: {mode}')

    return rdf
# ---------------------------------------------------------------------
