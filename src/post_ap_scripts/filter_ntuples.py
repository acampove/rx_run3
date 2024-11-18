#!/usr/bin/env python3
'''
Script used to filter ntuples produced by AP
'''

import argparse

from dmu.logging.log_store     import LogStore
from post_ap.ntuple_filter import ntuple_filter

log=LogStore.add_logger('dmu:post_ap_scripts:filter_ntuples')
#----------------------------------------
class Data:
    '''
    Class used to store shared data
    '''
    cfg_ver : str
    dset    : str
    ngroup  : int
    gindex  : int
    log_lv  : int
#----------------------------------------
def _set_log():
    LogStore.set_level('rx_scripts:atr_mgr:mgr',             30)
    LogStore.set_level('post_ap:FilterFile'   , Data.log_lv)
    LogStore.set_level('post_ap:ntuple_filter', Data.log_lv)
#----------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Will produce a smaller ntuple from a large one, for a given group of files')
    parser.add_argument('-c', '--cfg_ver', type=str, required=True , help='Type of job, e.g. comp')
    parser.add_argument('-d', '--dset'   , type=str, required=True , help='Dataset, e.g. dt_2024_turbo')
    parser.add_argument('-n', '--ngroup' , type=int, required=True , help='Number of groups of files')
    parser.add_argument('-i', '--gindex' , type=int, required=True , help='Index of the current group been processed')
    parser.add_argument('-l', '--loglvl' , type=int, required=False, help='Loglevel', default=20, choices=[10, 20, 30, 40])
    args = parser.parse_args()

    Data.cfg_ver= args.cfg_ver
    Data.dset   = args.dset
    Data.ngroup = args.ngroup
    Data.gindex = args.gindex
    Data.log_lv = args.loglvl
#----------------------------------------
def main():
    '''
    Execution starts here
    '''
    _get_args()
    _set_log()

    obj=ntuple_filter(dataset=Data.dset, cfg_ver=Data.cfg_ver, index=Data.gindex, ngroup=Data.ngroup)
    obj.filter()
#----------------------------------------
if __name__ == '__main__':
    main()
