'''
Module containing utility functions to be used with ROOT dataframes
'''

import re
from dataclasses import dataclass

import awkward as ak
import numpy

from ROOT import RDataFrame

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
def add_column(rdf : RDataFrame, arr_val : numpy.ndarray | None, name : str, d_opt : dict | None = None):
    '''
    Will take a dataframe, an array of numbers and a string
    Will add the array as a colunm to the dataframe

    d_opt (dict) : Used to configure adding columns
         exclude_re : Regex with patter of column names that we won't pick
    '''

    d_opt = {} if d_opt is None else d_opt
    if arr_val is None:
        raise ValueError('Array of values not introduced')

    if 'exclude_re' not in d_opt:
        d_opt['exclude_re'] = None

    v_col_org = rdf.GetColumnNames()
    l_col_org = [name.c_str() for name in v_col_org ]
    l_col     = []

    tmva_rgx  = r'tmva_\d+_\d+'

    for col in l_col_org:
        user_rgx = d_opt['exclude_re']
        if user_rgx is not None and re.match(user_rgx, col):
            log.debug(f'Dropping: {col}')
            continue

        if                          re.match(tmva_rgx, col):
            log.debug(f'Dropping: {col}')
            continue

        log.debug(f'Picking: {col}')
        l_col.append(col)

    data  = ak.from_rdataframe(rdf, columns=l_col)
    d_data= { col : data[col] for col in l_col }

    if arr_val.dtype == 'object':
        arr_val = arr_val.astype(float)

    d_data[name] = arr_val

    rdf = ak.to_rdataframe(d_data)

    return rdf
# ---------------------------------------------------------------------
