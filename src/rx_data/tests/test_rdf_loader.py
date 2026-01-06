'''
This module contains tests for the RDFLoader class
'''

from rx_common import Sample, Trigger
from rx_data   import RDFLoader
from rx_data   import SpecMaker

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
