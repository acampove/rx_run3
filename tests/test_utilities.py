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
# --------------------------------------------------
@pytest.mark.parametrize('event_type', Data.l_event_type)
def test_read_decay_name(event_type : str) -> None:
    '''
    Tests reading of decay name from YAML using event type
    '''
    literal = aput.read_decay_name(event_type=event_type, style='literal')
    safe_1  = aput.read_decay_name(event_type=event_type, style= 'safe_1')

    print(f'{literal:<50}{safe_1:<50}')
