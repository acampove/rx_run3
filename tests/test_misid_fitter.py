'''
This file contains tests for MisIDFitter
'''
import os
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dmu.stats.zfit            import zfit
from dmu.generic               import utilities  as gut
from dmu.stats.zfit_plotter    import ZFitPlotter
from zfit.core.interfaces      import ZfitData   as zdata
from zfit.core.interfaces      import ZfitPDF    as zpdf
from rx_misid.misid_fitter     import MisIDFitter
from rx_misid.misid_calculator import MisIDCalculator

# ---------------------------------------------------
class Data:
    '''
    Used to store attributes
    '''
    cache_dir = '/tmp/tests/rx_misid/misid_fitter'

    os.makedirs(cache_dir, exist_ok=True)
# ---------------------------------------------------
def _get_df(q2bin : str) -> pnd.DataFrame:
    data_path = f'{Data.cache_dir}/{q2bin}.parquet'
    if os.path.isfile(data_path):
        df = pnd.read_parquet(data_path)
        return df

    cfg = gut.load_data(package='rx_misid_data', fpath='misid.yaml')
    cfg['input']['q2bin'  ] = q2bin
    cfg['input']['sample' ] = 'DATA_24_MagUp_24c3'
    cfg['input']['project'] = 'rx'
    cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_ext'

    obj = MisIDCalculator(cfg=cfg, is_sig=False)
    df  = obj.get_misid()

    df.to_parquet(data_path)

    return df
# ---------------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    q2bin = 'low'
    data  = _get_toy_data()

    ftr   = MisIDFitter(data=data, q2bin=q2bin)
    pdf   = ftr.get_pdf()
