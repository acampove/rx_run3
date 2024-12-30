'''
Module with utility functions
'''
# pylint: disable=import-error, line-too-long

import os
import re
import glob
from importlib.resources     import files
from dataclasses             import dataclass
from dmu.logging.log_store   import LogStore
from ROOT                    import gSystem, gInterpreter, std, TString

log=LogStore.add_logger('rx_common:utilities')

# --------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''
    initialized : bool = False
    rgx_ldpath         = r'.*-L(\/[a-z]+\/.*\/lib).*'

    # TODO: Do not hardcode this
    yaml_config_path      = '/home/acampove/Tests/rx_samples'
    os.environ['LDFLAGS'] = '-L/home/acampove/Packages/ewp-rkstz-master-analysis/analysis/install/lib'
    os.environ['INCPATH'] = '/home/acampove/Packages/ewp-rkstz-master-analysis/analysis/install/include'

    cfg_inp  = {
            'nfiles'  : 10,
            'nentries': 100,
            'data_dir': '/tmp/test_tuple_holder',
            'sample'  : 'data_24_magdown_24c4',
            'hlt2'    : 'Hlt2RD_BuToKpEE_MVA'}
# --------------------------------
def get_config_dir() -> std.string:
    '''
    Will return path to directory where YAML configuration files are
    '''
    readm_path = files('rx_config').joinpath('README.md')
    readm_path = str(readm_path)
    config_dir = os.path.dirname(readm_path)
    config_dir = std.string(config_dir)

    return config_dir
# --------------------------------
def _load_library(name : str) -> None:
    '''
    Will load C++ libraries and include header files from RX framework
    '''

    lib_path = get_lib_path(name)
    log.debug(f'Loading: {lib_path}')
    gSystem.Load(lib_path)
# --------------------------------
def initialize_project(name : str) -> None:
    '''
    This function will:
    - Load libraries from underlying C++ project
    - Include header files from same project
    - Load configurations from YAML files, e.g. list of samples

    Parameters
    -----------------
    name : Name of project, used for library picking, e.g. kernel
    '''
    if Data.initialized:
        return

    _load_library(name)
    _include_headers()


    Data.initialized = True
# --------------------------------
def _include_headers() -> None:
    '''
    Will pick path to headers and include them
    '''
    log.debug('Including headers')

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
# -------------------------
def dict_to_map(d_data : dict[str,str]) -> std.map:
    '''
    Function taking a dictionary between strings
    and returning a C++ map between TStrings
    '''

    cpp_data = std.map("TString, TString")()
    for name, value in d_data.items():
        name  = TString(name)
        value = TString(value)

        cpp_data[name] = value

    return cpp_data
# -------------------------
