#!/usr/bin/env python3

from data_checks.ntuple_filter import ntuple_filter

import argparse

#----------------------------------------
class data:
    ngroup=None
    gindex=None
    config=None
#----------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Will produce a smaller ntuple from a large one, for a given group of files') 
    parser.add_argument('-n', '--ngroup' , type=int, help='Number of groups of files')
    parser.add_argument('-i', '--gindex' , type=int, help='Index of the current group been processed')
    parser.add_argument('-c', '--config' , type=str, help='Name of config file, without the extension')
    args = parser.parse_args()

    data.ngroup = args.ngroup
    data.gindex = args.gindex
    data.config = args.config
#----------------------------------------
def main():
    get_args()

    obj=ntuple_filter(cfg_nam=data.config, index=data.gindex, groups=data.ngroup)
    obj.filter()
#----------------------------------------
if __name__ == '__main__':
    main()

