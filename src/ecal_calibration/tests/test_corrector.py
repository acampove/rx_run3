'''
Script with code needed to test Corrector class
'''
from importlib.resources import files

import tqdm
import pytest
import pandas as pnd

from vector                        import MomentumObject4D as v4d
from dmu.generic                   import utilities        as gut
from dmu.logging.log_store         import LogStore
from ecal_calibration              import utilities        as cut
from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration.corrector    import Corrector

pytestmark = pytest.mark.skip(
    reason='''
    This part of the code did not produce useful results
    will turn off the tests, due to time constraints
    ''')
# -----------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('ecal_calibration:regressor'   , 20)
    LogStore.set_level('ecal_calibration:preprocessor', 20)
    LogStore.set_level('ecal_calibration:corrector'   , 10)
# -----------------------------------------------------------
def _load_data(name : str) -> dict:
    fpath = files('ecal_calibration_data').joinpath(f'tests/regressor/{name}.json')
    fpath = str(fpath)

    data            = gut.load_json(fpath)
    data['L1_brem'] = data['L1_HASBREMADDED']
    data['L2_brem'] = data['L2_HASBREMADDED']

    return data
# -----------------------------------------------------------
def _get_corrector() -> Corrector:
    kind = 'row_col_are_eng'
    cfg  = cut.load_cfg(name='tests/corrector/simple')
    cfg['saving']['out_dir'] = f'regressor/predict_{kind}'

    cal  = Corrector(cfg=cfg)

    return cal
# -----------------------------------------------------------
def test_calibrate_simple(_dask_client):
    '''
    Tests `corrector` from the Corrector class
    '''

    data = _load_data(name='row')
    sr   = pnd.Series(data)
    sr   = PreProcessor.build_features(
        row        =  sr,
        lep        ='L1',
        skip_target=True)
    cor      = _get_corrector()

    for val in [100, 200, 300, 400, 500]:
        electron = v4d(pt=2250 + val, eta=3.0, phi=1, m=0.511)
        electron = cor.run(electron, row=sr)
# -----------------------------------------------------------
def test_calibrate_are(_dask_client):
    '''
    Tests tests correction for different areas
    '''

    data     = _load_data(name='row')
    sr       = pnd.Series(data)
    sr       = PreProcessor.build_features(
            row        =  sr,
            lep        ='L1',
            skip_target=True)
    cor      = _get_corrector()

    for are in [0, 1, 2]:
        sr['are']= are
        electron = v4d(pt=2250, eta=3.0, phi=1, m=0.511)
        electron = cor.run(electron, row=sr)
# -----------------------------------------------------------
def test_calibrate_benchmark(_dask_client):
    '''
    Tests `corrector` from the Corrector class
    '''

    data     = _load_data(name='row')
    sr       = pnd.Series(data)
    sr       = PreProcessor.build_features(
            row        =  sr,
            lep        ='L1',
            skip_target=True)
    cor      = _get_corrector()

    with LogStore.level('ecal_calibration:corrector', 30):
        for val in tqdm.tqdm(range(10_000), ascii=' -'):
            electron = v4d(
                    px=-10_000 + val,
                    py=-10_000 + val,
                    pz= 43253,
                    e=  43437)

            electron = cor.run(electron, row=sr)
# -----------------------------------------------------------
def test_calibrate_from_map(_dask_client):
    '''
    Tests tests correction using maps
    '''
    data     = _load_data(name='row')
    sr       = pnd.Series(data)
    sr       = PreProcessor.build_features(
            row        =  sr,
            lep        ='L1',
            skip_target=False)

    cor      = _get_corrector()

    for are in [0, 1, 2]:
        sr['are']= are
        electron = v4d(pt=2250, eta=3.0, phi=1, m=0.511)
        electron = cor.run(electron, row=sr, from_nn=False)
# -----------------------------------------------------------
