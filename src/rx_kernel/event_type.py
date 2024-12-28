'''
Module containing python interface to C++ EventType 
'''
# pylint: disable=invalid-name

from ROOT   import EventType    as EventType_cpp 
from ROOT   import ConfigHolder as ConfigHolder_cpp 
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_common:event_type')

def EventType(cfg_cpp : ConfigHolder_cpp) -> EventType_cpp:
    '''
    Function taking a ConfigHolder instance 
    and returning EventType object implemented in c++ class
    '''

    evt_cpp = EventType_cpp(cfg_cpp)

    return evt_cpp
