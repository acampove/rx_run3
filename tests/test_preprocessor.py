'''
Module testing PreProcessor class
'''

from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration              import utilities as cut

# ---------------------------------------------
def test_simple():
    '''
    Simplest test for preprocessor
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf()

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    df  = pre.get_data()

    assert len(df) == 10000
# ---------------------------------------------
