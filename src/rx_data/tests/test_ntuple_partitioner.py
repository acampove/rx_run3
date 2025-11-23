'''
This module has functions to test the NtuplePartitioner class
'''
from rx_common import Project
from rx_data   import NtuplePartitioner

def test_simple():
    prt   = NtuplePartitioner(kind = 'main', project = Project.rk_no_pid)
    paths = prt.partition(index=3, total=10)

    assert isinstance(paths, set)
    assert paths
