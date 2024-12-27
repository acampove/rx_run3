'''
Module containing python interface to C++ ConfigHolder
'''
# pylint: disable=import-error, invalid-name

from typing import Union

from ROOT import ConfigHolder as ConfigHolder_cpp
from ROOT import TString
# ------------------------------------------------------------------
def _str_to_tstring(key : str, cfg : dict, fall_back : Union[str,None]) -> TString:
    if key in cfg:
        val = cfg[key]
        return TString(val)

    if fall_back is not None:
        return TString(fall_back)

    raise ValueError(f'Required argument {key} not found')
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
