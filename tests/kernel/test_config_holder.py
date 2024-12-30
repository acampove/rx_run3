'''
Module where config holder is tested
'''

from rx_kernel.config_holder import ConfigHolder

import pytest
from ROOT import MessageSvc
from dmu.logging.log_store import LogStore

MessageSvc.Initialize(-1)

# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_common:config_holder', 10)
# -----------------------------------
# -----------------------------------
def test_run12():
    '''
    Simplest test constructor taking strings for Run1 and Run2
    '''

    cfg = {
            'project'  : 'RK',
            'analysis' : 'EE',
            'sample'   : 'Bd2K2EE',
            'q2bin'    : 'central',
            'year'     : '24',
            'polarity' : 'MD',
            'trigger'  : 'L0L',
            'trg_cfg'  : 'exclusive',
            'brem'     : '0G',
            'track'    : 'LL',
            }

    ch = ConfigHolder(cfg=cfg, is_run3=False)
    ch.Print()
# -----------------------------------
def test_run3():
    '''
    Simplest test constructor taking strings for Run3
    '''

    cfg = {
            'project'  : 'RK',
            'analysis' : 'EE',
            'data_dir' : '/publicfs/ucas/user/campoverde/Data/RX_run3/v1',
            'sample'   : 'data_24_magdown_24c4',
            'hlt2'     : 'Hlt2RD_B0ToKpPimEE',
            'tree_name': 'DecayTree',
            'q2bin'    : 'central',
            'year'     : '24',
            'polarity' : 'MD',
            'brem'     : '0G',
            'track'    : 'LL',
            'cut_opt'  : '',
            'wgt_opt'  : '',
            'tup_opt'  : '',
            }


    ch = ConfigHolder(cfg=cfg)
    ch.Print()
# -----------------------------------
