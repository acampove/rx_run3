'''
Module where config holder is tested
'''

from rx_kernel.config_holder import ConfigHolder
from rx_kernel import allowed_conf

import pytest
from ROOT import MessageSvc

MessageSvc.Initialize(-1)

# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    allowed_conf.Initialize('/home/acampove/Tests/rx_samples')
# -----------------------------------
def test_default():
    '''
    Simplest test of default constructor
    '''
    ch = ConfigHolder()
    ch.Print()
# -----------------------------------
# -----------------------------------
def test_string_run12():
    '''
    Simplest test constructor taking strings for Run1 and Run2
    '''

    cfg = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'LPT',
            'q2bin'   : 'central',
            'year'    : '24',
            'polarity': 'MD',
            'trigger' : 'L0L',
            'trg_cfg' : 'exclusive',
            'brem'    : '0G',
            'track'   : 'LL',
            }

    ch = ConfigHolder(cfg=cfg)
    ch.Print()
# -----------------------------------
def test_string_run3():
    '''
    Simplest test constructor taking strings for Run3
    '''


    cfg = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'data_24_magdown_24c4',
            'q2bin'   : 'central',
            'year'    : '24',
            'polarity': 'MD',
            'brem'    : '0G',
            'track'   : 'LL',
            }

    ch = ConfigHolder(cfg=cfg)
    ch.Print()
# -----------------------------------
