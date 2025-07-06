'''
This file contains tests for MisIDFitter
'''
import os
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dmu.stats.zfit            import zfit
from dmu.generic               import utilities  as gut
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
def _validate_data(df : pnd.DataFrame, name : str) -> None:
    df.plot.scatter('L1_flag', 'L2_flag', s=100)
    plt.savefig(f'{Data.cache_dir}/{name}_pass_fail.png')
    plt.close()

    plt_path = f'{Data.cache_dir}/{name}.png'
    plt.savefig(plt_path)
    plt.close()
# ---------------------------------------------------
def _add_pass_flags(df : pnd.DataFrame, name : str) -> pnd.DataFrame:
    df[f'{name}_pass'] = df.eval(f'{name}_PROBNN_E > 0.2 & {name}_PID_E > 3.0').astype(int)
    df[f'{name}_fail'] = df.eval(f'{name}_PROBNN_E < 0.2 | {name}_PID_E < 3.0').astype(int)
    df[f'{name}_flag'] = df[f'{name}_pass'] - df[f'{name}_fail']

    return df
# ---------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str):
    '''
    Simplest test
    '''
    df      = _get_df(q2bin=q2bin)
    df      = _add_pass_flags(df=df, name='L1')
    df      = _add_pass_flags(df=df, name='L2')

    _validate_data(df=df, name=f'simple_{q2bin}')

    arr_wgt = df['weight'    ].to_numpy()
    arr_mas = df['B_Mass_smr'].to_numpy()
    obs     = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    data    = zfit.data.from_numpy(obs=obs, array=arr_mas, weights=arr_wgt)

    ftr     = MisIDFitter(data=data, q2bin=q2bin)
    pdf     = ftr.get_pdf()
# ---------------------------------------------------
