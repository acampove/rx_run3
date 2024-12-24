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

    obj = ConfigHolder_cpp(
            _str_to_tstring('project' , cfg, fall_back =     None),
            _str_to_tstring('analysis', cfg, fall_back =     None),
            _str_to_tstring('sample'  , cfg, fall_back =       ''),
            _str_to_tstring('q2bin'   , cfg, fall_back = 'global'),
            _str_to_tstring('year'    , cfg, fall_back = 'global'),
            _str_to_tstring('polarity', cfg, fall_back = 'global'),
            _str_to_tstring('trigger' , cfg, fall_back = 'global'),
            _str_to_tstring('trg_cfg' , cfg, fall_back = 'global'),
            _str_to_tstring('brem'    , cfg, fall_back = 'global'),
            _str_to_tstring('track'   , cfg, fall_back = 'global'))

    return obj
# ------------------------------------------------------------------
