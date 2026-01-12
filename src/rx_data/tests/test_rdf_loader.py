'''
This module contains tests for the RDFLoader class
'''
import pytest

from dask.distributed import Client, LocalCluster
from rx_common        import Qsq, Sample, Trigger
from rx_data          import RDFLoader
from rx_data          import SpecMaker
from rx_selection     import selection as sel
from dmu              import LogStore

log=LogStore.add_logger('rx_data:test_rdf_loader')
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:rdf_loader', 10)
# ----------------------
def test_from_conf():
    '''
    Test loading dataframe from JSON config file
    '''
    sample = Sample.bpkpee
    trigger= Trigger.rk_ee_os

    mkr    = SpecMaker(sample=sample, trigger=trigger)
    path   = mkr.get_spec_path(per_file=False)

    rdf    = RDFLoader.from_conf(
        ntries = 3,
        wait   = 10,
        path   = path)

    assert rdf.Count().GetValue() > 0
