'''
Module containing utility functions to be used with ROOT dataframes
'''

import numpy

from ROOT import RDataFrame

import ROOT

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:rdataframe:utilities')

# ---------------------------------------------------------------------
def add_column(rdf : RDataFrame, arr_val : numpy.ndarray, name : str):
    '''
    Will take a dataframe, an array of numbers and a string
    Will add the array as a colunm to the dataframe
    '''
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

    @ROOT.Numba.Declare(['int'], 'float')
    def get_val(index):
        return arr_val[index]

    rdf = rdf.Define(name, 'Numba::get_val(rdfentry_)')

    return rdf
# ---------------------------------------------------------------------
