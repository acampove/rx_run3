'''
Module with functions used to test EventTuple class
'''
# pylint: disable=import-error, wrong-import-order

from dataclasses import dataclass

from ROOT import MessageSvc

from dmu.logging.log_store import LogStore

MessageSvc.Initialize(-1)

log=LogStore.add_logger('rx_common:test_tuple_holder')

def test_event_type():
    '''
    Test of event type constructor with map argument
    '''

