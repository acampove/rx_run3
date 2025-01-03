'''
Module with functions used to test functions in decays/utilities.py
'''
import pytest

import ap_utilities.decays.utilities as aput

# --------------------------------------------------
class Data:
    '''
    Class used to store data needed by tests
    '''

    l_event_type = [
        '10000000',
        '10000010',
        '10000020',
        '10000021',
        '10000022',
        '10000023',
        '10000027',
        '10000030',
        '10002203',
        '10002213',
        '11100001',
        '11100003',
        '11100006',
        ]

    l_new_nick = [
        'tau_5mu_eq_DPC',
        'tau_Kphinu_KK_eq_DPC',
        'tau_mugamma_eq_DPC',
        'tau_mumnKplKmn_eq_DPC',
        'tau_mumnpiplpimn_eq_DPC',
        'tau_mumue_eq_OS_DPC',
        'tau_mumue_eq_OS_FromB_TC',
        'tau_mumue_eq_SS_DPC',
        'tau_mumue_eq_SS_FromB_TC',
        'tau_mumumu_eq_DPC',
        'tau_mumumu_eq_FromB',
        'tau_muphi_KK_eq_DPC',
        'tau_muphi_KK_eq_FromB',
        'tau_muplpimnpimn_eq_DPC',
        'tau_nupiplpi0_eegamma_eq_DPC',
        'tau_pimnpiplpimnnu_eq_DPC',
        'tau_pimnpiplpimnpi0nu_eq_DPC',
        'tau_piphinu_KK_eq_DPC',
            ]

    l_old_nick = [
            'Bd2D0XNuEE',
            'Bd2D0XNuMM',
            'Bd2DNuKstNuEE',
            'Bd2DPiEE',
            'Bd2DPiMM',
            'Bd2DstNuDPiKPiEE',
            'Bd2DstNuDPiKPiMM',
            'Bd2KPiEE',
            'Bd2KPiMM',
            'Bd2KstEE',
            'Bd2KstEE_central',
            'Bd2KstEE_high',
            'Bd2KstEE_low',
            'Bd2KstEEvNOFLT',
            'Bd2KstEEvPS',
            'Bd2KstEta_EEG',
            ]
# --------------------------------------------------
@pytest.mark.parametrize('event_type', Data.l_event_type)
def test_read_decay_name(event_type : str) -> None:
    '''
    Tests reading of decay name from YAML using event type
    '''
    literal = aput.read_decay_name(event_type=event_type, style='literal')
    safe_1  = aput.read_decay_name(event_type=event_type, style= 'safe_1')

    print(f'{literal:<50}{safe_1:<50}')
# --------------------------------------------------
@pytest.mark.parametrize('new_nick', Data.l_new_nick)
def test_read_event_type(new_nick: str) -> None:
    '''
    Tests reading of event type from YAML using new_nick 
    '''
    event_type = aput.read_event_type(nickname=new_nick)
    print(event_type)
# --------------------------------------------------
@pytest.mark.parametrize('new_nick', Data.l_new_nick)
def test_new_from_old(new_nick : str) -> None:
    '''
    Will test function taking returning new nickname style
    from old nickname style
    '''
    old_nick = aput.new_from_old_nick(new_nick)
