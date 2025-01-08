'''
Module with tests for CacheData class
'''
# pylint: disable = import-error

import os
import glob
from typing        import Union
from dataclasses   import dataclass

import pytest
from dmu.logging.log_store   import LogStore
from rx_selection.cache_data import CacheData

log = LogStore.add_logger('rx_selection:test_cache_data')
# ---------------------------------------------
@dataclass
class Data:
    '''
    Class used to share data
    '''
    l_sam_trg_mc : list[tuple[str,str]]
    l_sam_trg_dt : list[tuple[str,str]]

    data_version = 'v1'
    l_rk_trigger = [
            'Hlt2RD_BuToKpMuMu_MVA',
            'Hlt2RD_BuToKpEE_MVA',
            'SpruceRD_BuToHpMuMu',
            'SpruceRD_BuToHpEE',
            'SpruceRD_BuToKpMuMu',
            'SpruceRD_BuToKpEE',
            ]

    l_rkst_trigger = ['']
# ---------------------------------------------
def _triggers_from_mc_sample(sample_path : str, is_rk : bool) -> list[str]:
    if 'DATA_' in sample_path:
        return []

    l_trigger = Data.l_rk_trigger if is_rk else Data.l_rkst_trigger
    l_trig    = [ trig for trig in l_trigger if os.path.isdir(f'{sample_path}/{trig}') ]

    return l_trig
# ---------------------------------------------
def _has_files(sample_path : str, trigger : str) -> bool:
    file_wc = f'{sample_path}/{trigger}/*.root'
    l_path  = glob.glob(file_wc)

    return len(l_path) != 0
# ---------------------------------------------
def _get_mc_samples(is_rk : bool) -> list[tuple[str,str]]:
    if hasattr(Data, 'l_sam_trg_mc'):
        return Data.l_sam_trg_mc

    if 'DATADIR' not in os.environ:
        raise ValueError('DATADIR not found in environment')

    data_dir   = os.environ['DATADIR']
    sample_dir = f'{data_dir}/RX_run3/{Data.data_version}/post_ap'
    l_sam_trg  = []
    for sample_path in glob.glob(f'{sample_dir}/*'):
        l_trigger   = _triggers_from_mc_sample(sample_path, is_rk)

        for trigger in l_trigger:
            sample_name = os.path.basename(sample_path)
            if not _has_files(sample_path, trigger):
                log.warning(f'Cannot find any files for: {sample_name}/{trigger}')
                continue

            l_sam_trg.append((sample_name, trigger))

    nsample = len(l_sam_trg)
    log.info(f'Found {nsample} samples')

    Data.l_sam_trg_mc = l_sam_trg

    return Data.l_sam_trg_mc
# ---------------------------------------------
def _get_dt_samples(is_rk : bool) -> list[tuple[str,str]]:
    if hasattr(Data, 'l_sam_trg_dt'):
        return Data.l_sam_trg_dt

    if 'DATADIR' not in os.environ:
        raise ValueError('DATADIR not found in environment')

    l_trigger  = Data.l_rk_trigger if is_rk else Data.l_rkst_trigger
    data_dir   = os.environ['DATADIR']
    sample_dir = f'{data_dir}/RX_run3/{Data.data_version}/post_ap'
    l_sam_trg  = []
    for sample_path in glob.glob(f'{sample_dir}/DATA_*'):
        for trigger in l_trigger:
            sample_name = os.path.basename(sample_path)
            if not _has_files(sample_path, trigger):
                log.warning(f'Cannot find any files for: {sample_name}/{trigger}')
                continue

            l_sam_trg.append((sample_name, trigger))

    nsample = len(l_sam_trg)
    log.info(f'Found {nsample} samples')

    Data.l_sam_trg_dt = l_sam_trg

    return Data.l_sam_trg_dt
# ---------------------------------------------
def _override_parts(cfg : dict, sample : str) -> Union[None,dict]:
    if sample in [
            'Bs_phieta_eplemng_eq_Dalitz_DPC',
            'Bs_phipi0_eplemng_eq_Dalitz_DPC',
            'Bu_KplKplKmn_eq_sqDalitz_DPC',
            'Bu_KplpiplKmn_eq_sqDalitz_DPC',
            'Bu_Lambdacbarppi_Lambdabarmunu_eq_HELAMP_TC',
            'Bu_piplpimnKpl_eq_sqDalitz_DPC',
            'Bu_Kstgamma_Kst_eq_KSpi_DPC_SS',
            'Bd_K1gamma_Kpipi0_eq_mK1270_HighPtGamma_DPC',
            'Bd_Kstpi0_eq_TC_Kst982width100_HighPtPi0',
            'Bd_Dmnpipl_eq_DPC',
            'Bs_Phipi0_gg_eq_DPC_SS',
            'Bs_PhiEta_gg_eq_DPC_SS',
            ]:
        log.warning(f'Skipping sample {sample}')
        return None

    if sample in [
            'Bd_JpsiKS_ee_eq_CPV_DPC',
            'Bd_Ksteta_eplemng_eq_Dalitz_DPC',
            'Bu_D0pi_Kmunu_eq_DPC',
            'Bu_D0enu_Kpi_eq_DPC_TC',
            'Bd_Dstplenu_eq_PHSP_TC',
            'Bd_D0Xenu_D0_eq_cocktail',
            'Bs_Dsenu_phienu_eq_DPC_HVM_EGDWC',
            'Bu_phiKee_KK_eq_DPC']:
        cfg['npart'] = 10

    if sample in [
            'Bd_Ksteta_gg_eq_DPC_SS',
            'Bd_Kstgamma_eq_HighPtGamma_DPC',
            'Bu_D0munu_Kpi_eq_cocktail_D0muInAcc_BRcorr1',
            ]:
        cfg['npart'] = 1

    return cfg
# ---------------------------------------------
def _get_config(sample : str, trigger : str, is_rk : bool) -> dict:
    '''
    Takes name to config file
    Return settings from YAML as dictionary
    '''
    data_dir = os.environ['DATADIR']

    d_conf            = {}
    d_conf['ipart'  ] = 0
    d_conf['npart'  ] = 50
    d_conf['ipath'  ] = f'{data_dir}/RX_run3/v1/post_ap'
    d_conf['sample' ] = sample
    d_conf['project'] = 'RK' if is_rk else 'RKst'
    d_conf['q2bin'  ] = 'central'
    d_conf['hlt2'   ] = trigger
    d_conf['remove' ] = ['q2', 'bdt']

    return d_conf
# ---------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_mc_samples(is_rk=True))
def test_run3_rk_mc(sample : str, trigger : str):
    '''
    Testing on run3 RK samples and triggers
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = _get_config(sample, trigger, is_rk = True)

    cfg = _override_parts(cfg, sample)
    if cfg is None:
        return

    LogStore.set_level('rx_selection:ds_getter' , 10)
    LogStore.set_level('rx_selection:cache_data', 10)

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_dt_samples(is_rk=True))
def test_run3_rk_dt(sample : str, trigger : str):
    '''
    Testing on run3 RK samples and triggers
    '''
    log.info(f'{sample:<60}{trigger:<40}')
    cfg = _get_config(sample, trigger, is_rk = True)
    if cfg is None:
        return

    LogStore.set_level('rx_selection:ds_getter' , 10)
    LogStore.set_level('rx_selection:cache_data', 10)

    # This combination has a very low efficiency, do not limit number of files
    if sample != 'DATA_24_MagDown_24c1' and trigger != 'SpruceRD_BuToHpEE':
        cfg['max_files']  = 10

    obj=CacheData(cfg = cfg)
    obj.save()
# ---------------------------------------------
