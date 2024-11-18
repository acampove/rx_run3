'''
Script used to save PFNs
'''

import re
import logging
import argparse
from importlib.resources    import files

import apd
import dmu.generic.utilities as gut
from dmu.logging.log_store  import LogStore

import post_ap.utilities as utdc

log=LogStore.add_logger('post_ap:save_pfns')
#------------------------------------
class Data:
    '''
    Class used to store shared attributes
    '''
    config      :  str
    log_lvl     :  int
    local_config: bool
#------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Will use apd to save a list of paths to ROOT files in EOS')
    parser.add_argument('-c', '--config' , type=str, help='Name of config file, without TOML extension')
    parser.add_argument('-l', '--log_lvl', type=int, help='Logging level', default=20, choices=[10, 20, 30, 40])
    parser.add_argument('-L', '--loc_cfg', help='Will pick up local version of config, if not use, will pick up grid version', action='store_true')
    args = parser.parse_args()

    Data.config       = args.config
    Data.log_lvl      = args.log_lvl
    Data.local_config = args.loc_cfg
#------------------------------------
def _get_pfns() -> dict:
    '''
    Returns dictionary of PFS
    '''
    utdc.local_config = Data.local_config

    cfg_dat = utdc.load_config(Data.config)
    d_prod  = cfg_dat['production']

    log.debug('Reading paths from APD')
    obj     = apd.get_analysis_data(**d_prod)

    if   Data.config.startswith('dt_'):
        d_pfn = _get_dt_pfns(obj)
    elif Data.config.startswith('mc_'):
        d_pfn = _get_mc_pfns(obj)
    else:
        raise ValueError(f'Unrecognized start of config: {Data.config}')

    _print_pfn_info(d_pfn)

    return d_pfn
#------------------------------------
def _print_pfn_info(d_pfn : dict[str, list[str]]) -> None:
    nsample=0
    for prod, l_sam in d_pfn.items():
        nsample += len(l_sam)
        log.info(f'{prod:<50}{nsample:<20}')

    log.info(f'Found {nsample} PFNs')
#------------------------------------
def _get_dt_pfns(ap_obj) -> dict[str,list[str]]:
    '''
    Takes AP object and returns dictionary between name of production and list of PFNs
    '''
    cfg_dat  = utdc.load_config(Data.config)
    l_samp   = cfg_dat['sample']['names']
    d_sample = {samp : ap_obj(name=samp) for samp in l_samp}

    return d_sample
#------------------------------------
def _get_mc_pfns(ap_obj):
    '''
    Reads from AP object sample info and returns samplename -> PFNs dictionary
    '''
    cfg_dat   = utdc.load_config(Data.config)
    regex     = cfg_dat['sample']['name_rx']
    version   = cfg_dat['sample']['version']
    pattern   = re.compile(regex)
    collection= ap_obj.all_samples()

    d_path = {}
    for d_info in collection:
        name = d_info['name']
        vers = d_info['version']

        if vers != version:
            continue

        if not pattern.match(name):
            continue

        sam          = collection.filter(name=name)
        d_path[name] = sam.PFNs()

    return d_path
#------------------------------------
def _save_pfns(d_path):
    '''
    Save dictionary of samplename -> PFNs to JSON
    '''
    pfn_path= files('post_ap_data').joinpath(f'{Data.config}.json')
    log.info(f'Saving to: {pfn_path}')
    gut.dump_json(d_path, pfn_path)
#------------------------------------
def _set_log():
    LogStore.set_level('post_ap:save_pfns', Data.log_lvl)
    if Data.log_lvl == 10:
        LogStore.set_level('post_ap:utilities', Data.log_lvl)

        logging.basicConfig()
        log_apd=logging.getLogger('apd')
        log_apd.setLevel(Data.log_lvl)
#------------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    _set_log()
    d_pfn=_get_pfns()
    _save_pfns(d_pfn)
#------------------------------------
if __name__ == '__main__':
    main()
