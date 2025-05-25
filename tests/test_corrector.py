'''
Script with code needed to test the Corrector class
'''

from ecal_calibration.corrector import Corrector
from ecal_calibration           import utilities as cut

# -----------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    ddf = cut.get_ddf()
    cfg = cut.load_cfg(name='tests/corrector/simple')

    obj = Corrector(ddf=ddf, cfg=cfg)
    ddf = obj.get_ddf()
# -----------------------------------------
