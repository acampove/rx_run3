'''
Script holding functions needed to test SampleWeighter class
'''
import os

import numpy
import pytest
import matplotlib.pyplot as plt
import pandas            as pnd
from dmu.logging.log_store    import LogStore
from dmu.generic              import utilities      as gut
from rx_misid.sample_weighter import SampleWeighter

log=LogStore.add_logger('rx_misid:test_weighter')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    user     = os.environ['USER']
    out_dir  = f'/tmp/{user}/tests/rx_misid/sample_weighter'
    nentries = 100_000

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
    LogStore.set_level('rx_misid:sample_weighter', 20)
    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------------------------------
def _validate_weights(
    sample : str,
    df     : pnd.DataFrame,
    mode   : str,
    lep    : str) -> None:

    arr_pt = df[f'{lep}_TRACK_PT' ].to_numpy()
    arr_et = df[f'{lep}_TRACK_ETA'].to_numpy()
    arr_wt = df['weight'          ].to_numpy()

    arr_zr = arr_wt[arr_wt < 1e-6]
    nzeros = len(arr_zr)
    log.info(f'Zeroes= {nzeros}')

    _, (ax1, ax2, ax3) = plt.subplots(ncols=3, figsize=(20, 7))

    ax1.hist2d(arr_et, arr_pt, bins=50, cmap='viridis', vmin=0, vmax=60)
    ax1.set_xlabel(r'$\eta$')
    ax1.set_ylabel(r'$p_T$')
    ax1.set_title('Unweighted')

    ax2.hist2d(arr_et, arr_pt, bins=50, cmap='viridis', vmin=0, vmax=None, weights=arr_wt)
    ax2.set_xlabel(r'$\eta$')
    ax2.set_ylabel(r'$p_T$')
    ax2.set_title('Weighted')

    if sample in ['Bu_JpsiK_ee_eq_DPC', 'Bu_Kee_eq_btosllball05_DPC', 'DATA_24_MagUp_24c2']:
        rng = 0, 2.00
        bins= 3
    else:
        rng = 0, 0.01
        bins= 200

    ax3.hist(arr_wt, bins=bins, range=rng)
    ax3.set_xlabel('Weights')

    plt.tight_layout()
    plt.savefig(f'{Data.out_dir}/{mode}_{sample}_{lep}.png')
    plt.close()
# -------------------------------------------------------
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
        df[f'{lep}_TRACK_PT' ] = numpy.random.uniform(550, 20_000, Data.nentries)
        df[f'{lep}_TRACK_ETA'] = numpy.random.uniform(1.6, 4.0, Data.nentries)

    if not good_phase_space:
        return df

    df = df[ df['L1_TRACK_PT'] < (30_000 - 5_000 * df['L1_TRACK_ETA']) ]
    df = df[ df['L2_TRACK_PT'] < (30_000 - 5_000 * df['L2_TRACK_ETA']) ]

    df = df[ df['L1_TRACK_PT'] > ( 8_000 - 2_000 * df['L1_TRACK_ETA']) ]
    df = df[ df['L2_TRACK_PT'] > ( 8_000 - 2_000 * df['L2_TRACK_ETA']) ]

    return df
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

    wgt = SampleWeighter(
        df    = df,
        cfg   = cfg,
        sample= sample,
        is_sig= is_sig)
    df  = wgt.get_weighted_data()
    arr_block_out = df['block'].to_numpy()

    mode = {True : 'signal', False : 'control'}[is_sig]

    # Check that no shuffling of entries happened, using
    # array of block numbers
    assert numpy.array_equal(arr_block_inp, arr_block_out)

    assert 'pid_weights' in df.attrs

    _validate_weights(df=df, mode=mode, sample=sample, lep='L1')
    _validate_weights(df=df, mode=mode, sample=sample, lep='L2')
# ----------------------------
