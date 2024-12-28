'''
Module containing python interface to C++ EventType 
'''
# pylint: disable=invalid-name

from ROOT   import TString, std
from ROOT   import EventType as EventType_cpp 

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_common:event_type')

def EventType(cfg : dict[str,str]) -> EventType_cpp:
    '''
    Function taking configuration in dictionary
    and returning EventType object implemented in c++ class
    '''
    cfg_cpp = std.map("TString, TString")()
    for name, value in cfg.items():
        name  = TString(name)
        value = TString(value)

        cfg_cpp[name] = value

    evt_cpp = EventType_cpp(cfg_cpp)

    return evt_cpp
