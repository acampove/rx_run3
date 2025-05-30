'''
Script with code needed to test Calibration class
'''
import numpy
import matplotlib.pyplot as plt

from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration.regressor    import Regressor
from ecal_calibration              import utilities as cut

# -----------------------------------------------------------
def _plot_targets(pred : numpy.ndarray, real : numpy.ndarray) -> None:
    plt.hist(real, bins=60, range=[-2, 3], label='Real'     , alpha   =   0.3)
    plt.hist(pred, bins=60, range=[-2, 3], label='Predicted', histtype='step')

    plt.show()
# -----------------------------------------------------------
def test_flat_bias():
    '''
    Simplest test for calibration with biased data
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')

    ddf = cut.get_ddf(bias=1.1, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    obj = Regressor(ddf=ddf, cfg=cfg)
    obj.train()
# -----------------------------------------------------------
def test_loader():
    '''
    Tests loading existing model
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')

    ddf = cut.get_ddf(bias=1.1, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    obj = Regressor(ddf=ddf, cfg=cfg)
    obj.load()
# -----------------------------------------------------------
