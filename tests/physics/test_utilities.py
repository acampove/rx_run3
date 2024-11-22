'''
Module with functions used to test functions in physics/utilities.py
'''
import pytest

import dmu.physics.utilities as phut

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
            ]

    l_style = ['literal', 'safe_1']
# --------------------------------------------------
@pytest.mark.parametrize('event_type', Data.l_event_type)
@pytest.mark.parametrize(     'style', Data.l_style)
def test_read_decay_name(event_type : str, style : str) -> None:
    '''
    Tests reading of decay name from YAML using event type
    '''
    phut.read_decay_name(event_type=event_type, style=style)
