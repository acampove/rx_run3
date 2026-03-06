'''
Module with tests for SamplePrinter class
'''

import pytest

from dmu       import LogStore
from rx_data   import SamplesPrinter
from rx_common import Project

log=LogStore.add_logger('rx_data:test_samples_printer')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:samples', 10)
# ----------------------
def test_simple():
    ptr = SamplesPrinter(project = Project.rk)
    ptr.print_by_block()
# ----------------------
