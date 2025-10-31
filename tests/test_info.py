'''
This module tests the functions in the info.py module
'''

import pytest
from ROOT      import RDataFrame, RDF
from rx_common import info

# ----------------------
def _get_rdf(is_data : bool) -> RDF.RNode:
    '''
    Parameters
    -------------
    is_data: True if dataframe returned should represent real data

    Returns
    -------------
    ROOT dataframe
    '''
    rdf = RDataFrame(100)

    if not is_data:
        rdf = rdf.Define('B_TRUEID' , '521')
        rdf = rdf.Define('L1_TRUEID',  '11')
        rdf = rdf.Define('L2_TRUEID',  '11')

    rdf = rdf.Define('B_M', '5280')

    return rdf
# -----------------------------------------
def test_is_ee():
    '''
    Test that trigger is recognized as electron trigger
    '''
    assert info.is_ee(trigger='Hlt2RD_BuToKpEE_MVA')
    assert info.is_ee(trigger='Hlt2RD_BuToKpEE_MVA_noPID')
    assert info.is_ee(trigger='Hlt2RD_BuToKpEE_SameSign_MVA')
    assert info.is_ee(trigger='Hlt2RD_BuToKpEE_MVA_misid')

    assert not info.is_ee(trigger='Hlt2RD_BuToKpMuMu_MVA')
    assert not info.is_ee(trigger='Hlt2RD_BuToKpMuMu_MVA_noPID')
    assert not info.is_ee(trigger='Hlt2RD_BuToKpMuMu_SameSign_MVA')
    assert not info.is_ee(trigger='Hlt2RD_B0ToKpPimMuMu_MVA')
    assert not info.is_ee(trigger='Hlt2RD_B0ToKpPimMuMu_SameSign_MVA')
# -----------------------------------------
def test_is_reso():
    '''
    Tests is_reso
    '''
    assert not info.is_reso(q2bin='low')
    assert not info.is_reso(q2bin='central')
    assert not info.is_reso(q2bin='high')

    assert info.is_reso(q2bin='jpsi')
    assert info.is_reso(q2bin='psi2')
# -----------------------------------------
@pytest.mark.parametrize('channel,trigger', [
    ('EE', 'Hlt2RD_BuToKpEE_MVA'),
    ('EE', 'Hlt2RD_BuToKpEE_MVA_noPID'),
    ('EE', 'Hlt2RD_BuToKpEE_SameSign_MVA'),
    ('EE', 'Hlt2RD_BuToKpEE_MVA_misid'),
    ('EE', 'Hlt2RD_BuToKpEE_MVA_ext'),
    ('EE', 'Hlt2RD_BuToKpEE_MVA_cal'),
    ('MM', 'Hlt2RD_BuToKpMuMu_MVA'),
    ('MM', 'Hlt2RD_BuToKpMuMu_MVA_noPID'),
    ('MM', 'Hlt2RD_BuToKpMuMu_SameSign_MVA'),
    ('EE', 'Hlt2RD_B0ToKpPimEE_MVA'),
    ('EE', 'Hlt2RD_B0ToKpPimEE_MVA_noPID'),
    ('EE', 'Hlt2RD_B0ToKpPimEE_SameSign_MVA'),
    ('EE', 'Hlt2RD_B0ToKpPimEE_MVA_misid'),
    ('EE', 'Hlt2RD_B0ToKpPimEE_MVA_ext'),
    ('EE', 'Hlt2RD_B0ToKpPimEE_MVA_cal'),
    ('MM', 'Hlt2RD_B0ToKpPimMuMu_MVA'),
    ('MM', 'Hlt2RD_B0ToKpPimMuMu_MVA_noPID'),
    ('MM', 'Hlt2RD_B0ToKpPimMuMu_SameSign_MVA')])
def test_channel_from_trigger(channel : str, trigger : str):
    '''
    Test for function providing EE/MM for a given HLT2 trigger
    '''
    assert channel == info.channel_from_trigger(trigger=trigger)
# -----------------------------------------
@pytest.mark.parametrize('project,trigger', [
    ('RK_noPID'  , 'Hlt2RD_BuToKpEE_MVA_noPID'),
    ('RK_noPID'  , 'Hlt2RD_BuToKpMuMu_MVA_noPID'),
    ('RKst_noPID', 'Hlt2RD_B0ToKpPimEE_MVA_noPID'),
    ('RKst_noPID', 'Hlt2RD_B0ToKpPimMuMu_MVA_noPID'),
    # --------
    ('RK', 'Hlt2RD_BuToKpEE_MVA'),
    ('RK', 'Hlt2RD_BuToKpEE_SameSign_MVA'),
    ('RK', 'Hlt2RD_BuToKpEE_MVA_misid'),
    ('RK', 'Hlt2RD_BuToKpEE_MVA_ext'),
    ('RK', 'Hlt2RD_BuToKpEE_MVA_cal'),
    ('RK', 'Hlt2RD_BuToKpMuMu_MVA'),
    ('RK', 'Hlt2RD_BuToKpMuMu_SameSign_MVA'),
    ('RKst', 'Hlt2RD_B0ToKpPimEE_MVA'),
    ('RKst', 'Hlt2RD_B0ToKpPimEE_SameSign_MVA'),
    ('RKst', 'Hlt2RD_B0ToKpPimEE_MVA_misid'),
    ('RKst', 'Hlt2RD_B0ToKpPimEE_MVA_ext'),
    ('RKst', 'Hlt2RD_B0ToKpPimEE_MVA_cal'),
    ('RKst', 'Hlt2RD_B0ToKpPimMuMu_MVA'),
    ('RKst', 'Hlt2RD_B0ToKpPimMuMu_SameSign_MVA')])
def test_project_from_trigger(project : str, trigger : str):
    '''
    Test for function providing RK/RKst for a given HLT2 trigger
    '''

    assert project == info.project_from_trigger(trigger=trigger, lower_case=False)

    project = project.lower()

    assert project == info.project_from_trigger(trigger=trigger, lower_case=True)
# -----------------------------------------
@pytest.mark.parametrize('is_data', [True, False])
def test_is_rdf_data(is_data : bool) -> None:
    '''
    Tests is_rdf_data 
    '''
    rdf = _get_rdf(is_data=is_data)

    assert is_data == info.is_rdf_data(rdf=rdf)
# -----------------------------------------
