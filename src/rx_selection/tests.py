'''
Module containing functions needed by tests
'''
import os
import glob
from dataclasses   import dataclass

from dmu.logging.log_store   import LogStore

log = LogStore.add_logger('rx_selection:tests')

# ---------------------------------------------
@dataclass
class Data:
    '''
    Class used to share data
    '''
    l_sam_trg_mc : list[tuple[str,str]]
    l_sam_trg_dt : list[tuple[str,str]]

    data_version = 'v2'
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
def get_mc_samples(is_rk : bool, included : str = '') -> list[tuple[str,str]]:
    '''
    Will return list of samples, where a sample is a pair of sample name and trigger
    Will only pick samples whose name include the `included` substring
    '''
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
    log.warning(f'Found {nsample} samples in {sample_dir}')

    Data.l_sam_trg_mc = l_sam_trg

    if included == '':
        return Data.l_sam_trg_mc

    l_sam_trg = [ (sam, trg) for (sam, trg) in l_sam_trg if included in sam ]

    return l_sam_trg
# ---------------------------------------------
def get_config(sample : str, trigger : str, is_rk : bool) -> dict:
    '''
    Takes name to config file
    Return settings from YAML as dictionary
    Used for CacheData tests
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
def get_dsg_config(sample : str, trigger : str, is_rk : bool) -> dict:
    '''
    Function will return config file for DsGetter tests
    '''
    cfg        = get_config(sample, trigger, is_rk)
    l_remove   = cfg['remove']
    d_redefine = { rem : '(1)' for rem in l_remove}

    del cfg['remove']

    cfg['redefine'] = d_redefine
    cfg['cutver']   = 'v1'

    return cfg
# ---------------------------------------------
