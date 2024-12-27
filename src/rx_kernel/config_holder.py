'''
Module containing python interface to C++ ConfigHolder
'''
# pylint: disable=import-error, invalid-name
import os

from typing import Union

from ROOT import ConfigHolder as ConfigHolder_cpp
from ROOT import TString
from ROOT import std

# ------------------------------------------------------------------
def _check_datadir(cfg : dict) -> None:
    if 'DATADIR' not in cfg:
        raise KeyError('Setting not found: DATADIR')

    data_dir = cfg['DATADIR']
    if not os.path.isdir(data_dir):
        raise FileNotFoundError(f'Cannot find: {data_dir}')
# ------------------------------------------------------------------
def ConfigHolder(cfg : Union[dict,None] = None) -> ConfigHolder_cpp:
    '''
    This function creates the ConfigHolder object and returns it
    '''
    if cfg is None:
        return ConfigHolder_cpp()

    _check_datadir(cfg)

    cpp_cfg= std.map('TString, TString')()
    for name, value in cfg.items():
        name = TString(value)
        value= TString(value)

        if name == 'DATADIR':
            os.environ[name] = value
        else:
            cpp_cfg[name]=value

    obj = ConfigHolder_cpp(cpp_cfg)

    return obj
# ------------------------------------------------------------------
