'''
Module containing functions needed by tests
'''
# pylint: disable=import-error

import os
import glob
from typing        import Union
from dataclasses   import dataclass
from functools     import cache

from dmu.logging.log_store   import LogStore

log = LogStore.add_logger('rx_selection:tests')
# ---------------------------------------------
@dataclass
class Data:
    '''
    Class used to share data
    '''
    data_version = 'v3'
    l_rk_trigger = [
            'Hlt2RD_BuToKpMuMu_MVA',
            'Hlt2RD_BuToKpEE_MVA',
            'SpruceRD_BuToHpMuMu',
            'SpruceRD_BuToHpEE',
            ]

    l_rkst_trigger = ['']
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
def _has_files(sample_path : str, trigger : str) -> bool:
    file_wc = f'{sample_path}/{trigger}/*.root'
    l_path  = glob.glob(file_wc)

    return len(l_path) != 0
# ---------------------------------------------
def _triggers_from_mc_sample(sample_path : str, is_rk : bool) -> list[str]:
    if 'DATA_' in sample_path:
        return []

    l_trigger = Data.l_rk_trigger if is_rk else Data.l_rkst_trigger
    l_trig    = [ trig for trig in l_trigger if os.path.isdir(f'{sample_path}/{trig}') ]

    return l_trig
# ---------------------------------------------
@cache
def get_dt_samples(is_rk : bool) -> list[tuple[str,str]]:
    '''
    Will return list of data samples
    Where a sample is a pair of sample name and trigger name
    '''
    if 'DATADIR' not in os.environ:
        raise ValueError('DATADIR not found in environment')

    l_trigger  = Data.l_rk_trigger if is_rk else Data.l_rkst_trigger
    data_dir   = os.environ['DATADIR']
    sample_dir = f'{data_dir}/RX_run3/{Data.data_version}/post_ap'
    l_dir      = glob.glob(f'{sample_dir}/DATA_*')
    l_sam_trg  = []

    if len(l_dir) == 0:
        raise ValueError(f'No directories found in: {sample_dir}')

    for sample_path in l_dir:
        for trigger in l_trigger:
            sample_name = os.path.basename(sample_path)
            if not _has_files(sample_path, trigger):
                log.warning(f'Cannot find any files for: {sample_name}/{trigger}')
                continue

            l_sam_trg.append((sample_name, trigger))

    return l_sam_trg 
# ---------------------------------------------
@cache
def get_mc_samples(is_rk : bool) -> list[tuple[str,str]]:
    '''
    Will return list of samples, where a sample is a pair of sample name and trigger
    '''
    if 'DATADIR' not in os.environ:
        raise ValueError('DATADIR not found in environment')

    data_dir   = os.environ['DATADIR']
    sample_dir = f'{data_dir}/RX_run3/{Data.data_version}/post_ap'
    l_dir      = glob.glob(f'{sample_dir}/*')
    l_sam_trg  = []

    if len(l_dir) == 0:
        raise ValueError(f'No directories found in: {sample_dir}')

    for sample_path in l_dir:
        l_trigger   = _triggers_from_mc_sample(sample_path, is_rk)

        for trigger in l_trigger:
            sample_name = os.path.basename(sample_path)
            if not _has_files(sample_path, trigger):
                log.warning(f'Cannot find any files for: {sample_name}/{trigger}')
                continue

            l_sam_trg.append((sample_name, trigger))

    return l_sam_trg 
# ---------------------------------------------
def get_config(sample : str, trigger : str, is_rk : bool, remove : list) -> Union[dict, None]:
    '''
    Takes name to config file
    Return settings from YAML as dictionary
    Used for CacheData tests
    '''
    data_dir = os.environ['DATADIR']

    d_conf            = {}
    d_conf['ipart'  ] = 0
    d_conf['npart'  ] = 50 if not sample.startswith('DATA_') else 10000
    d_conf['ipath'  ] = f'{data_dir}/RX_run3/{Data.data_version}/post_ap'
    d_conf['sample' ] = sample
    d_conf['project'] = 'RK' if is_rk else 'RKst'
    d_conf['q2bin'  ] = 'central'
    d_conf['hlt2'   ] = trigger
    d_conf['remove' ] = remove

    d_conf = _override_parts(d_conf, sample)

    return d_conf
# ---------------------------------------------
def get_dsg_config(sample : str, trigger : str, is_rk : bool, remove : list) -> Union[None,dict]:
    '''
    Function will return config file for DsGetter tests
    '''
    cfg = get_config(sample, trigger, is_rk, remove)
    if cfg is None:
        return None

    l_remove   = cfg['remove']
    d_redefine = { rem : '(1)' for rem in l_remove}

    del cfg['remove']

    cfg['redefine'] = d_redefine
    cfg['cutver']   = 'v1'

    return cfg
# ---------------------------------------------
