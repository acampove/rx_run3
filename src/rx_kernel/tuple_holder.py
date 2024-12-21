'''
Module containing python interface to C++ TupleHolder
'''
from typing import Union
from ROOT   import TString
from ROOT   import ConfigHolder as ConfigHolder_cpp
from ROOT   import TupleHolder  as TupleHolder_cpp

# pylint: disable=invalid-name
# -----------------------------------------------------------
def TupleHolder(
        cfg     : Union[ConfigHolder_cpp,None] = None,
        options : Union[str,None]              = None) -> TupleHolder_cpp:
    '''
    Function returning TupleHolder c++ implementation's instance
    '''
    if cfg is None and options is None:
        return TupleHolder_cpp()

    opt = TString(options)
    obj = TupleHolder_cpp(cfg, opt)

    return obj
# -----------------------------------------------------------
