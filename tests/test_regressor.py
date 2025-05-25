'''
Script with code needed to test Calibration class
'''

from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration.regressor    import Regressor
from ecal_calibration              import utilities as cut

# -----------------------------------------------------------
def test_simple():
    '''
    Simplest test for calibration
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')

    ddf = cut.get_ddf()
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    df  = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    obj = Regressor(df=df, cfg=cfg)
    obj.train()
# -----------------------------------------------------------
