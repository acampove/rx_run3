'''
Module where config holder is tested
'''

from rx_kernel.config_holder import ConfigHolder
from rx_kernel import allowed_conf

import pytest
from ROOT import MessageSvc
from dmu.logging.log_store import LogStore

MessageSvc.Initialize(-1)

# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    allowed_conf.Initialize('/home/acampove/Tests/rx_samples')
    LogStore.set_level('rx_common:config_holder', 10)
# -----------------------------------
def test_default():
    '''
    Simplest test of default constructor
    '''
    ch = ConfigHolder()
    ch.Print()
# -----------------------------------
# -----------------------------------
def test_run12():
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
def test_run3():
    '''
    Simplest test constructor taking strings for Run3
    '''

    cfg = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'data_24_magdown_24c4',
            'hlt2'    : 'Hlt2RD_B0ToKpPimEE',
            'q2bin'   : 'central',
            'year'    : '24',
            'polarity': 'MD',
            'trigger' : '',   # This will be left empty for Run3, where the L0 trigger does not exist
            'brem'    : '0G',
            'track'   : 'LL',
            'data_dir': '/publicfs/ucas/user/campoverde/Data/RX_run3/v1',
            }

    ch = ConfigHolder(cfg=cfg)
    ch.Print()
# -----------------------------------
