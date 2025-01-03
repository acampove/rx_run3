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
            'Bd_Dmnpipl_eq_DPC',
            'Bd_Dmnpipl_eq_DPC',
            'Bd_Dstplenu_eq_PHSP_TC',
            'Bd_Dstplmunu_eq_PHSP_TC',
            'Bd_Kpimumu_eq_DPC',
            'Bd_Kpimumu_eq_DPC',
            'Bd_Kstee_eq_btosllball05_DPC',
            'Bd_Kstee_eq_btosllball05_DPC',
            'Bd_Kstee_eq_btosllball05_DPC',
            'Bd_Kstee_eq_btosllball05_DPC',
            'Bd_Kstee_eq_DPC',
            'Bd_Kstee_flatq2_eq_DPC_MomCut',
            'Bd_Ksteta_eplemng_eq_Dalitz_DPC',
            ]

    l_old_nick = [
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
@pytest.mark.parametrize('old_nick', Data.l_old_nick)
def test_new_from_old(old_nick : str) -> None:
    '''
    Will test function returning new nickname style
    from old nickname style
    '''
    old_nick = aput.new_from_old_nick(old_nick)
    print(old_nick)
# --------------------------------------------------
@pytest.mark.parametrize('new_nick', Data.l_new_nick)
def test_old_from_new(new_nick : str) -> None:
    '''
    Will test function returning old nickname style
    from new nickname style
    '''
    old_nick = aput.old_from_new_nick(new_nick)
    print(old_nick)
