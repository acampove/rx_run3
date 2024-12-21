'''
Module containing python interface to C++ ConfigHolder
'''
from typing import Union

from ROOT import ConfigHolder as ConfigHolder_cpp
from ROOT import TString
# ------------------------------------------------------------------
def _str_to_tstring(key : str, cfg : dict) -> TString:
    val = cfg[key]

    return TString(val)
# ------------------------------------------------------------------
def ConfigHolder(cfg : Union[dict,None] = None) -> ConfigHolder_cpp:
    '''
    This function creates the ConfigHolder object and returns it
    '''
    if cfg is None:
        return ConfigHolder_cpp()

    obj = ConfigHolder_cpp(
            _str_to_tstring('project' , cfg),
            _str_to_tstring('analysis', cfg),
            _str_to_tstring('sample'  , cfg),
            _str_to_tstring('q2bin'   , cfg),
            _str_to_tstring('year'    , cfg),
            _str_to_tstring('polarity', cfg),
            _str_to_tstring('trigger' , cfg),
            _str_to_tstring('trg_cfg' , cfg),
            _str_to_tstring('brem'    , cfg),
            _str_to_tstring('track'   , cfg))

    return obj
# ------------------------------------------------------------------

