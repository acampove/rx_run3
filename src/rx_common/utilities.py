'''
Module with utility functions 
'''
# pylint: disable=import-error

import os
import re
import glob
from dataclasses           import dataclass

from dmu.logging.log_store import LogStore
from ROOT                  import gSystem, gInterpreter

log=LogStore.add_logger('rx_common:utilities')
# --------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''
    rgx_ldpath = r'.*-L(\/[a-z]+\/.*\/lib).*'

    os.environ['LDFLAGS'] = '-L/home/acampove/Packages/ewp-rkstz-master-analysis/analysis/install/lib'
    os.environ['INCPATH'] = '/home/acampove/Packages/ewp-rkstz-master-analysis/analysis/install/include'
# --------------------------------
def include_headers() -> None:
    '''
    Will pick path to headers and include them
    '''
    inc_path = os.environ['INCPATH']
    l_header = glob.glob(f'{inc_path}/*.hpp')

    for header_path in l_header:
        gInterpreter.ProcessLine(f'#include "{header_path}"')
# --------------------------------
def get_lib_path(lib_name : str) -> str:
    '''
    Takes name of library, e.g. kernel
    Returns path to it
    '''
    ld_arg  = os.environ['LDFLAGS']
    mtch    = re.match(Data.rgx_ldpath, ld_arg)
    if not mtch:
        raise ValueError(f'Cannot extract libraries path from: {ld_arg}')

    ld_path    = mtch.group(1)
    lib_path   = f'{ld_path}/lib{lib_name}.so'
    if not os.path.isfile(lib_path):
        raise FileNotFoundError(f'Cannot find: {lib_path}')

    return lib_path
# --------------------------------
def load_library(lib_path : str) -> None:
    '''
    Takes path to library and loads it
    '''
    log.debug(f'Loading: {lib_path}')
    gSystem.Load(lib_path)
# --------------------------------
