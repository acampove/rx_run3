'''
Module used to test EfficiencyScanner class
'''
import numpy

from rx_efficiencies.efficiency_scanner import EfficiencyScanner as EffSc

def test_scan():
    '''
    Test efficiency scanning
    '''
    cfg = {
            'input' : {
                'sample' : 'Bu_JpsiK_ee_eq_DPC',
                'trigger': 'Hlt2RD_BuToKpEE_MVA'},
            'variables' : {
                'mva.mva_cmb' : numpy.arange(0.5, 1.0, 0.1),
                'mva.mva_prc' : numpy.arange(0.5, 1.0, 0.1),
                }
            }

    obj = EffSc(cfg=cfg)
    df  = obj.run()
