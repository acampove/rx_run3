'''
This module contains functions to test ScaleCombiner
'''
import pytest
import pandas as pnd

from rx_q2                 import ScaleCombiner
from dmu.logging.log_store import LogStore

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_q2:scale_combiner', 10)
# ----------------------
def _check_df(df : pnd.DataFrame) -> None:
    '''
    Parameters
    -------------
    df: Dataframe with scales
    '''
    l_block = df['block'].tolist()
    s_block = set(l_block)

    assert s_block == {1, 2, 3, 4, 5, 6, 7, 8}

    assert not df.isna().any().any()
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    cmb   = ScaleCombiner(version = 'v3')
    df_ee = cmb.combine(name='parameters_ee.json', measurements=['rk_ee', 'rkst_ee'])
    _check_df(df=df_ee)

    df_mm = cmb.combine(name='parameters_mm.json', measurements=['rk_mm', 'rkst_mm'])
    _check_df(df=df_mm)
# ----------------------
