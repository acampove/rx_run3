'''
Script holding functions needed to test SampleWeighter class
'''
from functools import cache, lru_cache
import os

import pickle
import numpy
import pytest
import pandas            as pnd
from dmu.logging.log_store    import LogStore
from dmu.generic              import utilities      as gut
from rx_misid.sample_weighter import FloatArray, SampleWeighter

log=LogStore.add_logger('rx_misid:test_weighter')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    user     = os.environ['USER']
    out_dir  = f'/tmp/{user}/tests/rx_misid/sample_weighter'
    nentries = 5_000

    l_block = [
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8]
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    numpy.random.seed(42)
    LogStore.set_level('rx_misid:sample_weighter', 10)
    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------------------------------
@lru_cache(maxsize=None)
def _get_dataframe(good_phase_space :  bool = True) -> pnd.DataFrame:
    df           = pnd.DataFrame(index=range(Data.nentries))
    df['hadron'] = numpy.random.choice(['kaon' ,   'pion'], size=Data.nentries)
    df['kind'  ] = numpy.random.choice(['PassFail', 'FailPass', 'FailFail'], size=Data.nentries)
    df['block' ] = numpy.random.choice(Data.l_block, size=Data.nentries)
    df['weight'] = numpy.random.choice([1, 10], size=Data.nentries)

    df['L1_HASBREM' ] = numpy.random.choice(a = [0, 1], p = [0.5, 0.5], size = Data.nentries)
    df['L2_HASBREM' ] = numpy.random.choice(a = [0, 1], p = [0.5, 0.5], size = Data.nentries)
    df['L1_PROBNN_E'] = numpy.random.random(size=Data.nentries)
    df['L2_PROBNN_E'] = numpy.random.random(size=Data.nentries)
    df['L1_PID_E'   ] = numpy.random.uniform(-10, 10, size=Data.nentries)
    df['L2_PID_E'   ] = numpy.random.uniform(-10, 10, size=Data.nentries)

    for lep in ['L1', 'L2']:
        df[f'{lep}_TRACK_PT' ] = 10 ** numpy.random.uniform(2, 5, Data.nentries)
        df[f'{lep}_TRACK_ETA'] = numpy.random.uniform(2.0,     5.5, Data.nentries)

    if not good_phase_space:
        return df

    df = df[ df['L1_TRACK_PT'] < (30_000 - 5_000 * df['L1_TRACK_ETA']) ]
    df = df[ df['L2_TRACK_PT'] < (30_000 - 5_000 * df['L2_TRACK_ETA']) ]

    df = df[ df['L1_TRACK_PT'] > ( 8_000 - 2_000 * df['L1_TRACK_ETA']) ]
    df = df[ df['L2_TRACK_PT'] > ( 8_000 - 2_000 * df['L2_TRACK_ETA']) ]

    return df
# ----------------------
def _check_weights(df : pnd.DataFrame) -> None:
    '''
    Parameters
    -------------
    df: Pandas dataframe to be checked
    '''
    
    assert 'pid_weights' in df.attrs
    assert 'pid_eff_l1'  in df.attrs
    assert 'pid_eff_l2'  in df.attrs

    arr_wgt_tot = numpy.array(df.attrs['pid_weights'])
    arr_wgt_l1  = numpy.array(df.attrs['pid_eff_l1' ])
    arr_wgt_l2  = numpy.array(df.attrs['pid_eff_l2' ])

    assert numpy.isclose(arr_wgt_tot, arr_wgt_l1 * arr_wgt_l2, rtol=1e-5).all()
# ----------------------
def _check_blocks(
    df            : pnd.DataFrame,
    arr_block_inp : FloatArray) -> None:
    '''
    Parameters
    -------------
    df           : DataFrame to check
    arr_block_inp: Array of blocks in the input data 
    '''
    arr_block_out = df['block'].to_numpy()

    # Check that no shuffling of entries happened, using
    # array of block numbers
    assert numpy.array_equal(arr_block_inp, arr_block_out)
# ----------------------------
@pytest.mark.parametrize('is_sig', [True, False])
@pytest.mark.parametrize('sample', [
    'DATA_24_MagUp_24c2',
    'Bu_JpsiK_ee_eq_DPC',
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_KplKplKmn_eq_sqDalitz_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC'])
def test_simple(is_sig : bool, sample : str):
    '''
    Parameters
    -------------
    is_sig: If true, it will weight sample to put it in the signal region
            Otherwise it will go to control region
    sample: MC and data samples need to be weighted in different ways
    '''
    cfg = gut.load_conf(package='rx_misid_data', fpath='weights.yaml')
    df  = _get_dataframe(good_phase_space=False)
    arr_block_inp = df['block'].to_numpy()

    if not sample.startswith('DATA_'):
        df = df.drop(columns=['kind'])

    cfg.plots_path = Data.out_dir

    wgt = SampleWeighter(
        df    = df,
        cfg   = cfg,
        sample= sample,
        is_sig= is_sig)
    df  = wgt.get_weighted_data()

    _check_blocks(df=df, arr_block_inp=arr_block_inp)
    _check_weights(df=df)
# ----------------------------
