'''
Module holding tests for SampleEmulator class
'''

import numpy
import pytest
from ROOT                    import RDataFrame, RDF # type: ignore
from rx_data.sample_emulator import SampleEmulator

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:test_sample_emulator')

_SAMPLES_MOTHER_SWAP=[
    'Bs_JpsiKst_mm_eq_DPC' ,
    'Bs_JpsiKst_ee_eq_DPC' ,
]

_SAMPLES_HADRON_SWAP=[
    'Bd_JpsiKst_mm_had_swp',
    'Bd_JpsiKst_ee_had_swp',
]

_SAMPLE_PAIRS = [
    ('Bs_JpsiKst_mm_eq_DPC' , 'Bd_JpsiKst_mm_eq_DPC'),
    ('Bs_JpsiKst_ee_eq_DPC' , 'Bd_JpsiKst_ee_eq_DPC'),
]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:sample_emulator', 10)
    LogStore.set_level('dmu:generic:utilities'  , 10)
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
    rdf = rdf.Define('H1_TRUEID'           , '321')
    rdf = rdf.Define('H2_TRUEID'           , '211')
    rdf = rdf.Define('H1_PT'               , '10000')
    rdf = rdf.Define('H2_PT'               , '20000')

    return rdf
# --------------------------------------------
@pytest.mark.parametrize('new_name, old_name', _SAMPLE_PAIRS)
def test_rename(
    new_name : str, 
    old_name : str):
    '''
    Check renaming of samples

    new_name: Sample requested 
    old_name: Sample that will emulate sample requested 
    '''
    emu = SampleEmulator(sample=new_name)
    val = emu.get_sample_name()

    assert old_name == val 
# --------------------------------------------
@pytest.mark.parametrize('sample', _SAMPLES_MOTHER_SWAP)
def test_swap_mother(sample : str):
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
# --------------------------------------------
@pytest.mark.parametrize('sample', _SAMPLES_HADRON_SWAP)
def test_swap_hadrons(sample : str):
    '''
    sample: Name of sample to emulate
    '''
    log.info('')
    rdf_old = _get_rdf()
    emu     = SampleEmulator(sample=sample)
    rdf_new = emu.post_process(rdf = rdf_old)

    data_old = rdf_old.AsNumpy()
    data_new = rdf_new.AsNumpy()
    for variable in ['TRUEID', 'PT']:
        arr_1o= data_old[f'H1_{variable}']
        arr_2o= data_old[f'H2_{variable}']

        arr_1n= data_new[f'H1_{variable}']
        arr_2n= data_new[f'H2_{variable}']

        assert numpy.isclose(arr_1o, arr_2n, rtol=1e-5).all()
        assert numpy.isclose(arr_2o, arr_1n, rtol=1e-5).all()
