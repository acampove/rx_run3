'''
Module with tests for SamplePrinter class
'''

from rx_data   import SamplesPrinter
from rx_common import Project

def test_simple():
    ptr = SamplesPrinter(project = Project.rk)
    ptr.print_by_block()
