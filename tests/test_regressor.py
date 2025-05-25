'''
Script with code needed to test Calibration class
'''

from ecal_calibration.regressor import Regressor
from ecal_calibration           import utilities as cut

def test_simple():
    '''
    Simplest test for calibration
    '''
    ddf = cut.get_ddf()
    cfg = cut.load_cfg(name='tests/regressor/simple')

    obj = Regressor(ddf=ddf, cfg=cfg)
    obj.train()
    obj.test()
