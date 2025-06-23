'''
Module used to test EfficiencyScanner class
'''
import numpy
import pytest

from dmu.logging.log_store import LogStore
from rx_efficiencies.efficiency_scanner import EfficiencyScanner as EffSc

log = LogStore.add_logger('rx_efficiencies:test_efficiency_scanner')
# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_efficiencies:efficiency_scanner', 10)
# -----------------------------------
def test_scan():
    '''
    Test efficiency scanning
    '''
    cfg = {
            'input' : 
            {
                'sample' : 'Bu_JpsiK_ee_eq_DPC',
                'trigger': 'Hlt2RD_BuToKpEE_MVA',
                'q2bin'  : 'jpsi',
                },
            'variables' : 
            {
                'mva_cmb' : numpy.arange(0.5, 1.0, 0.1),
                'mva_prc' : numpy.arange(0.5, 1.0, 0.1),
                }
            }

    obj = EffSc(cfg=cfg)
    df  = obj.run()
# -----------------------------------
