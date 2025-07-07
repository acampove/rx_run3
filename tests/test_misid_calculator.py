'''
Script meant to test MisIDCalculator class
'''
import os

import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dmu.generic               import utilities as gut
from dmu.logging.log_store     import LogStore
from rx_selection              import selection as sel
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:test_misid_calculator')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir     = '/tmp/tests/rx_misid/misid_calculator'
    config_name = 'misid.yaml'
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:misid_calculator', 10)
    LogStore.set_level('rx_misid:sample_splitter' , 10)
    LogStore.set_level('rx_misid:sample_weighter' , 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# ---------------------------------
def _get_config() -> dict:
    cfg = gut.load_data(package='rx_misid_data', fpath='misid.yaml')

    return cfg
# ---------------------------------
def _validate_df(
        df     : pnd.DataFrame,
        sample : str,
        q2bin  : str,
        test   : str,
        mode   : str) -> None:
    log.info(f'Validating {test} {sample} {q2bin} {mode}')

    _, (ax1, ax2) = plt.subplots(ncols=2, figsize=(20, 10))

    if not sample.startswith('DATA_'):
        ax1.hist(df['B_Mass_smr'      ], bins=50, histtype='step', range=(4500, 6000), weights=df['weight'], label=    'Smeared')

    ax1.hist(df['B_M_brem_track_2'], bins=50, histtype='step', range=(4500, 6000), weights=df['weight'], label='Non-smeared')
    ax1.axvline(x=5280, label=r'$B^+$', ls=':', c='red')
    ax1.legend()
    ax1.set_title(f'{sample}; {mode}; {q2bin}')

    if sample   == 'Bu_piplpimnKpl_eq_sqDalitz_DPC':
        rng = 0.0, 0.004 # PID weights will be small for misID MC
        bins= 80
    elif sample in ['Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC']:
        rng = 0.0, 2.0 # These are true electrons, high efficiencies
        bins= 2
    elif sample.startswith('DATA_') and mode == 'signal':
        rng = 0.0, 0.1 # Data transfer weights should be small
        bins= 50
    elif sample.startswith('DATA_') and mode == 'control':
        rng = 0.0, 11. # Data transfer weights should be small
        bins= 50
    else:
        raise ValueError(f'Invalid sample: {sample}')

    ax2.hist(df['weight'], bins=bins, histtype='step', range=rng)

    os.makedirs(f'{Data.out_dir}/{test}', exist_ok=True)

    plt.savefig(f'{Data.out_dir}/{test}/{sample}_{mode}_{q2bin}.png')
    plt.close()
# ---------------------------------
@pytest.mark.parametrize('q2bin' , ['low', 'central'])
@pytest.mark.parametrize('mode'  , ['signal', 'control'])
@pytest.mark.parametrize('sample', ['DATA_24_MagUp_24c3', 'Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC'])
def test_non_misid(sample : str, mode : str, q2bin : str):
    '''
    Simplest example of misid calculator with different samples
    '''
    cfg                    = _get_config()
    cfg['input']['sample'] = sample
    cfg['input']['q2bin' ] = q2bin

    if   sample.startswith('DATA'):
        cfg['input']['project'] = 'rx'
        cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_ext'
    else:
        cfg['input']['project'] = 'nopid'
        cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_noPID'

    is_sig = {'signal' : True, 'control' : False}[mode]

    obj = MisIDCalculator(cfg=cfg, is_sig=is_sig)
    df  = obj.get_misid()

    _validate_df(df=df, sample=sample, mode=mode, test='non_misid', q2bin=q2bin)
# ---------------------------------
@pytest.mark.parametrize('q2bin' , ['low', 'central'])
@pytest.mark.parametrize('mode'  , ['signal', 'control'])
@pytest.mark.parametrize('sample', ['Bu_piplpimnKpl_eq_sqDalitz_DPC'])
def test_misid(sample : str, mode : str, q2bin : str):
    '''
    Test calculator with misID samples, for noPID trigger
    '''
    cfg                     = _get_config()
    cfg['input']['sample' ] = sample
    cfg['input']['q2bin'  ] = q2bin
    cfg['input']['project'] = 'nopid'
    cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_noPID'

    is_sig = {'signal' : True, 'control' : False}[mode]

    obj = MisIDCalculator(cfg=cfg, is_sig=is_sig)
    df  = obj.get_misid()

    _validate_df(df=df, sample=sample, mode=mode, test='misid', q2bin=q2bin)
# ---------------------------------
@pytest.mark.parametrize('q2bin'   , ['low', 'central'])
@pytest.mark.parametrize('mode'    , ['signal', 'control'])
@pytest.mark.parametrize('sample'  , ['Bu_piplpimnKpl_eq_sqDalitz_DPC', 'DATA_24_MagUp_24c3'])
@pytest.mark.parametrize('has_brem', [True, False])
def test_misid_by_brem(
        has_brem : bool,
        sample   : str,
        mode     : str,
        q2bin    : str):
    '''
    Test calculator with misID samples, for noPID trigger and separately
    for candidates with brem and without brem assigned
    '''
    cfg                     = _get_config()
    cfg['input']['sample' ] = sample
    cfg['input']['q2bin'  ] = q2bin
    if sample.startswith('DATA'):
        cfg['input']['project'] = 'rx'
        cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_ext'
    else:
        cfg['input']['project'] = 'nopid'
        cfg['input']['trigger'] = 'Hlt2RD_BuToKpEE_MVA_noPID'

    is_sig   = {'signal' : True, 'control' : False}[mode]
    brem_cut = 'nbrem > 0' if has_brem else 'nbrem == 0'

    with sel.custom_selection(d_sel={'nbrem' : brem_cut}):
        obj = MisIDCalculator(cfg=cfg, is_sig=is_sig)
        df  = obj.get_misid()

    brem_status = {True : 'with_brem', False : 'no_brem'}[has_brem]

    _validate_df(df=df, sample=sample, mode=mode, test=brem_status, q2bin=q2bin)
# ---------------------------------
