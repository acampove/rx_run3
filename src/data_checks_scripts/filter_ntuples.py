#!/usr/bin/env python3

from data_checks.ntuple_filter import ntuple_filter
from log_store                 import log_store

import argparse

#----------------------------------------
class data:
    cfg_ver=None
    dset   =None
    ngroup =None
    gindex =None
    log_lv =None
#----------------------------------------
def set_log():
    log_store.set_level('rx_scripts:atr_mgr:mgr',             30)
    log_store.set_level('data_checks:FilterFile'   , data.log_lv)
    log_store.set_level('data_checks:ntuple_filter', data.log_lv)
#----------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Will produce a smaller ntuple from a large one, for a given group of files') 
    parser.add_argument('-c', '--cfg_ver', type=str, required=True , help='Type of job, e.g. comp')
    parser.add_argument('-d', '--dset'   , type=str, required=True , help='Dataset, e.g. dt_2024_turbo')
    parser.add_argument('-n', '--ngroup' , type=int, required=True , help='Number of groups of files')
    parser.add_argument('-i', '--gindex' , type=int, required=True , help='Index of the current group been processed')
    parser.add_argument('-l', '--loglvl' , type=int, required=False, help='Loglevel', default=20, choices=[10, 20, 30, 40])
    args = parser.parse_args()

    data.cfg_ver= args.cfg_ver
    data.dset   = args.dset
    data.ngroup = args.ngroup
    data.gindex = args.gindex
    data.log_lv = args.loglvl
#----------------------------------------
def main():
    get_args()
    set_log()

    obj=ntuple_filter(dataset=data.dset, cfg_ver=data.cfg_ver, index=data.gindex, ngroup=data.ngroup)
    obj.filter()
#----------------------------------------
if __name__ == '__main__':
    main()

