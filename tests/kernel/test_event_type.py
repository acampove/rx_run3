'''
Module with functions used to test EventTuple class
'''
# pylint: disable=import-error, wrong-import-order

from rx_kernel.event_type       import EventType
from rx_kernel.test_utilities   import get_config_holder
from dmu.logging.log_store      import LogStore

from ROOT import MessageSvc
MessageSvc.Initialize(-1)

log=LogStore.add_logger('rx_common:test_event_type')

def test_config_constructor():
    '''
    Test of event type constructor with ConfigHolder arg 
    '''
    cpp_cfg = get_config_holder(is_run3=True)
    evt     = EventType(cpp_cfg)
    evt.Init()
