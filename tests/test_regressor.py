'''
Script with code needed to test Calibration class
'''
import numpy
import pytest
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
@pytest.mark.parametrize('bias', [0.5, 0.8, 1.0, 1.2, 1.4])
def test_constant_predict(bias : float):
    '''
    Meant to test everything around network by:

    - Introducing data with constant (not dependent on features) bias
    - Training a constant model that outputs that bias
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(bias=bias, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    cfg['train']['epochs']   = 600
    cfg['saving']['out_dir'] = 'regressor/constant_predict'

    obj = Regressor(ddf=ddf, cfg=cfg)
    obj.train(constant_target=bias)

    pred= obj.predict(features=pre.features)

    assert numpy.allclose(pred, 1./bias, rtol=1e-4)
# -----------------------------------------------------------
    #_plot_targets(pred=pred, real=real)
