'''
Module with functions used to test functions in physics/utilities.py
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

    l_nickname = [
        'tau_5mu_eq_DPC',
        'tau_Kphinu_KK_eq_DPC',
        'tau_mugamma_eq_DPC',
        'tau_mumnKplKmn_eq_DPC',
        'tau_mumnpiplpimn_eq_DPC',
        'tau_mumue_eq_OS_DPC',
        'tau_mumue_eq_OS_FromB_TightCut',
        'tau_mumue_eq_SS_DPC',
        'tau_mumue_eq_SS_FromB_TightCut',
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
@pytest.mark.parametrize('nickname', Data.l_nickname)
def test_read_event_type(nickname: str) -> None:
    '''
    Tests reading of event type from YAML using nickname 
    '''
    event_type = aput.read_event_type(nickname=nickname, style= 'safe_1')
    print(event_type)
# --------------------------------------------------
