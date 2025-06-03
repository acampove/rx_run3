'''
Script with code needed to test Corrector class
'''
from importlib.resources import files

import dask
import pandas as pnd

from vector                        import MomentumObject4D as v4d
from dmu.generic                   import utilities        as gut
from ecal_calibration              import utilities        as cut
from ecal_calibration.preprocessor import PreProcessor
from ecal_calibration.corrector    import Corrector

# -----------------------------------------------------------
def _load_data(name : str) -> dict:
    fpath = files('ecal_calibration_data').joinpath(f'tests/regressor/{name}.json')
    fpath = str(fpath)
    data  = gut.load_json(fpath)

    return data
# -----------------------------------------------------------
def test_calibrate():
    '''
    Tests `calibrate_electron` from the Corrector class
    '''
    kind     = 'row_col_are_eng'

    electron = v4d(px=2250, py=-3287, pz=43253, e=43437)
    data     = _load_data(name='row')
    sr       = pnd.Series(data)
    df       = sr.to_frame()
    df       = df.T
    df['L1_brem'] = df['L1_HASBREMADDED']
    df['L2_brem'] = df['L2_HASBREMADDED']

    ddf_in = dask.dataframe.from_pandas(df)

    cfg = cut.load_cfg(name='tests/preprocessor/simple')
    pre = PreProcessor(ddf=ddf_in, cfg=cfg)
    ddf = pre.get_data()

    cfg      = cut.load_cfg(name='tests/regressor/simple')
    cfg['saving']['out_dir'] =f'regressor/predict_{kind}'

    cal      = Corrector(cfg=cfg)
    electron = cal.run(electron, ddf=ddf)
# -----------------------------------------------------------
