'''
Module containing python interface to C++ ConfigHolder
'''
# pylint: disable=import-error, invalid-name
import os

from ROOT import ConfigHolder as ConfigHolder_cpp
from ROOT import TString
from ROOT import std

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_common:config_holder')
# ------------------------------------------------------------------
def _check_datadir(cfg : dict) -> None:
    if 'data_dir' not in cfg:
        raise KeyError('Setting not found: data_dir')

    data_dir = cfg['data_dir']
    if not os.path.isdir(data_dir):
        raise FileNotFoundError(f'Cannot find: {data_dir}')
# ------------------------------------------------------------------
def _set_entry(name : str, cfg : dict[str,str]) -> None:
    if name not in cfg:
        cfg[name] = ''
# ------------------------------------------------------------------
def ConfigHolder(cfg : dict = None, is_run3 : bool = True) -> ConfigHolder_cpp:
    '''
    This function creates the ConfigHolder object and returns it

    Parameters
    ------------------
    cfg : Dictionary with configuration, which is optional.
    is_run3: By default it is true, will enforce check of attributes neede for Run3
    '''

    if is_run3:
        _check_datadir(cfg)
        _set_entry('trigger'  , cfg)
    else:
        _set_entry('tree_name', cfg)
        _set_entry('data_dir' , cfg)
        _set_entry('sample'   , cfg)
        _set_entry('hlt2'     , cfg)
        _set_entry('cut_opt'  , cfg)
        _set_entry('wgt_opt'  , cfg)
        _set_entry('tup_opt'  , cfg)

    cpp_cfg= std.map('TString, TString')()
    for name, value in cfg.items():
        name = TString(name)
        value= TString(value)
        cpp_cfg[name]=value
        log.debug(f'Setting: {name}')
        log.debug(f'Value  : {value}')
        log.debug('')

    obj = ConfigHolder_cpp(cpp_cfg)

    return obj
# ------------------------------------------------------------------
