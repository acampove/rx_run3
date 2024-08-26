#!/usr/bin/env python3

from importlib.resources import files
from log_store           import log_store

import apd
import argparse
import utils_noroot          as utnr
import data_checks.utilities as utdc

log=log_store.add_logger('data_checks:save_pfns')
#------------------------------------
class data:
    config = None
#------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Will use apd to save a list of paths to ROOT files in EOS')
    parser.add_argument('-c', '--config' , type=str, help='Name of config file, without TOML extension')
    args = parser.parse_args()

    data.config = args.config
#------------------------------------
def save_pfns():
    cfg_dat = utdc.load_config(data.config)
    d_samp  = cfg_dat['sample']
    d_prod  = cfg_dat['production']

    obj     = apd.get_analysis_data(**d_prod)
    l_path  = obj(**d_samp)
    l_path.sort()

    pfn_path= files('data_checks_data').joinpath(f'pfns/{data.config}.json')
    log.info(f'Saving to: {pfn_path}')
    utnr.dump_json(l_path, pfn_path)
#------------------------------------
def main():
    get_args()
    save_pfns()
#------------------------------------
if __name__ == '__main__':
    main()
