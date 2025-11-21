'''
Module with tests for SamplePrinter class
'''

from rx_data.samples  import SamplesPrinter
from rx_common.types  import Project

def test_simple():
    ptr = SamplesPrinter(project = Project.rk)
    ptr.print_by_block()
