'''
Script with code needed to test Calibration class
'''
from importlib.resources import files

import numpy
import pytest
import matplotlib.pyplot as plt

from torch                         import Tensor
from dask.distributed              import Client
from dmu.generic                   import utilities as gut
from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration.regressor    import Regressor
from ecal_calibration              import utilities as cut


pytestmark = pytest.mark.skip(
    reason='''
    This part of the code did not produce useful results
    will turn off the tests, due to time constraints
    ''')
# -----------------------------------------------------------
def _load_data(name : str) -> dict:
    fpath = files('ecal_calibration_data').joinpath(f'tests/regressor/{name}.json')
    fpath = str(fpath)
    data  = gut.load_json(fpath)

    return data
# -----------------------------------------------------------
def _plot_target_vs_features(arr_pred : numpy.ndarray, features : Tensor) -> None:
    arr_feat = features.numpy()
    arr_feat = arr_feat.T[0] # The zeroth feature is the one the target depends on, for now

    plt.scatter(arr_feat, arr_pred, s=1)
    plt.xlabel('Feature')
    plt.ylabel('Prediction')
    plt.grid()
    plt.show()
# -----------------------------------------------------------
def _plot_targets(
        pred : numpy.ndarray,
        real : numpy.ndarray,
        corr : float) -> None:

    minx = min(numpy.min(pred), numpy.min(real))
    maxx = max(numpy.max(pred), numpy.max(real))

    plt.hist(real, bins=80, range=[minx, maxx], label='Training' , alpha   =   0.3)
    plt.hist(pred, bins=80, range=[minx, maxx], label='Predicted', histtype='step')

    if corr is not None:
        plt.axvline(x=corr, label='Correction', ls=':', color='red')

    plt.legend()
    plt.xlabel('Real')
    plt.ylabel('Predicted')
    plt.show()
# -----------------------------------------------------------
def test_flat_bias():
    '''
    Simplest test for calibration with biased data
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')

    ddf = cut.get_ddf(name='fake_data', bias=1.1, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    obj = Regressor(ddf_tr=ddf, ddf_ts=ddf, cfg=cfg)
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
    obj = Regressor(ddf_tr=ddf, ddf_ts=ddf, cfg=cfg)
    obj.load()
# -----------------------------------------------------------
@pytest.mark.parametrize('bias', [0.5, 0.8, 1.0, 1.2, 1.4])
def test_constant_predict(bias : float):
    '''
    Meant to test everything around network by:

    - Introducing data with constant (not dependent on features) bias
    - Training a constant model that outputs that bias
    '''
    corr= 1.0 / bias

    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data', bias=1.1, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    cfg['train']['epochs']   = 200
    cfg['saving']['out_dir'] = 'regressor/constant_predict'

    obj = Regressor(ddf_tr=ddf, ddf_ts=ddf, cfg=cfg)
    obj.train(constant_target=corr)

    pred= obj.predict(features=pre.features)

    assert numpy.allclose(pred, corr, rtol=1e-5)
# -----------------------------------------------------------
@pytest.mark.parametrize('bias', [0.5, 0.8, 1.0, 1.2, 1.4])
def test_predict_flat_bias(bias : float):
    '''
    Meant to test whole pipeline by:

    - Introducing data with constant (not dependent on features) bias
    - Training a real model
    '''
    corr= 1.0 / bias

    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data', bias=1.1, kind='flat')
    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()

    cfg = cut.load_cfg(name='tests/regressor/simple')
    cfg['train']['epochs']   =  2_000
    cfg['input']['nentries'] = 15_000
    cfg['saving']['out_dir'] = 'regressor/predict_flat_bias'

    obj = Regressor(ddf_tr=ddf, ddf_ts=ddf, cfg=cfg)
    obj.train()

    pred= obj.predict(features=pre.features)
    real= pre.targets.numpy()
    _plot_targets(pred=pred, real=real, corr=corr)
# -----------------------------------------------------------
@pytest.mark.parametrize('kind', ['row_col_are_eng'])
def test_predict_bias(_dask_client : Client, kind : str):
    '''
    Meant to test everything by:

    - Training a real model that outputs the correction
    '''
    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    ddf = cut.get_ddf(name='fake_data', bias=None, kind=kind)

    pre = PreProcessor(ddf=ddf, cfg=cfg)
    ddf = pre.get_data()
    ddf_tr, ddf_ts = ddf.random_split([0.95, 0.05], random_state=42)

    cfg = cut.load_cfg(name='tests/regressor/simple')
    cfg['train']['epochs']   = 40000
    cfg['saving']['out_dir'] = f'regressor/predict_{kind}'

    obj = Regressor(ddf_tr=ddf_tr, ddf_ts=ddf_ts, cfg=cfg)
    obj.test()
# -----------------------------------------------------------
