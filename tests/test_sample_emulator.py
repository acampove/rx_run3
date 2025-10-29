'''
Module holding tests for SampleEmulator class
'''

import numpy
import pytest
from ROOT                    import RDataFrame, RDF # type: ignore
from rx_data.sample_emulator import SampleEmulator

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:test_sample_emulator')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:sample_emulator', 10)
    LogStore.set_level('dmu:generic:utilities'  , 20)
# ----------------------
def _get_rdf() -> RDF.RNode:
    '''
    Returns
    -------------
    Dataframe with zeroed columns 
    '''
    rdf = RDataFrame(100)
    rdf = rdf.Define('B_M'                 ,   '0')
    rdf = rdf.Define('B_Mass'              ,   '0')
    rdf = rdf.Define('B_M_brem_track_2'    ,   '0')
    rdf = rdf.Define('B_const_mass_M'      ,   '0')
    rdf = rdf.Define('B_const_mass_psi2S_M',   '0')
    rdf = rdf.Define('B_TRUEID'            , '123')
    rdf = rdf.Define('B_ID'                , '123')
    rdf = rdf.Define('Kst_MC_MOTHER_ID'    , '123')
    rdf = rdf.Define('Jpsi_MC_MOTHER_ID'   , '123')

    return rdf
# --------------------------------------------
@pytest.mark.parametrize('old_name, new_name',
    [
       ('Bs_JpsiKst_ee_eq_DPC', 'Bd_JpsiKst_ee_eq_DPC'),
       ('Bs_JpsiKst_mm_eq_DPC', 'Bd_JpsiKst_mm_eq_DPC'),
    ])
def test_rename(
    old_name : str, 
    new_name : str):
    '''
    Check renaming of samples

    old_name: Sample requested
    new_name: Expected sample to use
    '''
    emu = SampleEmulator(sample=old_name)
    val = emu.get_sample_name()

    assert new_name == val 
# --------------------------------------------
@pytest.mark.parametrize('sample', ['Bs_JpsiKst_ee_eq_DPC'])
def test_post_process(sample : str):
    '''
    sample: Name of sample to emulate
    '''
    log.info('')
    rdf_old = _get_rdf()
    emu     = SampleEmulator(sample=sample)
    rdf_new = emu.post_process(rdf = rdf_old)

    data    = rdf_new.AsNumpy()
    expected= 87.23
    for name, arr_val in data.items():
        if name.endswith('ID'):
            continue

        assert numpy.isclose(arr_val, expected).all()
