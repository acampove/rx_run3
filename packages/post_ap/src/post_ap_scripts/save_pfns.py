'''
Script used to save PFNs
'''

import os
import logging
import argparse

import dmu.generic.utilities as gut
from dmu.logging.log_store  import LogStore
from post_ap.pfn_reader     import PFNReader

import post_ap.utilities as utdc

log=LogStore.add_logger('post_ap:save_pfns')
#------------------------------------
class Data:
    '''
    Class used to store shared attributes
    '''
    production  :  str
    nickname    :  str
    config      :  str
    log_lvl     :  int
#------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Will use apd to save a list of paths to ROOT files in EOS')
    parser.add_argument('-p', '--production' , type=str, help='Name of production, e.g. rd_2024_ap')
    parser.add_argument('-n', '--nickname'   , type=str, help='Name of group of samples to process e.g. data')
    parser.add_argument('-c', '--config'     , type=str, help='Path to YAML config file')
    parser.add_argument('-l', '--log_lvl'    , type=int, help='Logging level', default=20, choices=[10, 20, 30, 40])
    args = parser.parse_args()

    Data.production   = args.production
    Data.nickname     = args.nickname
    Data.config       = args.config
    Data.log_lvl      = args.log_lvl
#------------------------------------
def _initialize() -> None:
    if not os.path.isfile(Data.config):
        raise FileNotFoundError(f'Cannot find: {Data.config}')

    os.environ['CONFIG_PATH'] = Data.config

    _set_log()
#------------------------------------
def _set_log() -> None:
    LogStore.set_level('post_ap:save_pfns', Data.log_lvl)
    if Data.log_lvl == 10:
        LogStore.set_level('post_ap:utilities', Data.log_lvl)

        logging.basicConfig()
        log_apd=logging.getLogger('apd')
        log_apd.setLevel(Data.log_lvl)
#------------------------------------
def _get_pfns() -> dict[str,list[str]]:
    '''
    Returns dictionary of sample -> PFNs
    '''
    cfg_dat = utdc.load_config()
    reader  = PFNReader(cfg=cfg_dat)
    d_pfn   = reader.get_pfns(production=Data.production, nickname=Data.nickname)

    return d_pfn
#------------------------------------
def _save_pfns(d_path : dict[str, list[str]]) -> None:
    '''
    Save dictionary of samplename -> PFNs to JSON
    '''

    pfn_path = f'./{Data.production}_{Data.nickname}.json'
    log.info(f'Saving to: {pfn_path}')
    gut.dump_json(d_path, pfn_path)
#------------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    d_pfn=_get_pfns()
    _save_pfns(d_pfn)
#------------------------------------
if __name__ == '__main__':
    main()
