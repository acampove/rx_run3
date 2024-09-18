#!/usr/bin/python3
'''
Script used to link ntuples properly and merge them
'''

import argparse

from dataclasses import dataclass

from log_store import log_store

log = log_store.add_logger('rx:data_checks:link_merge')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    job : str
# ---------------------------------
def _get_args():
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-j', '--job', type=str, help='Job name, e.g. flt_001', required=True) 
    args = parser.parse_args()

    Data.job = args.job
# ---------------------------------
def _get_paths():
    '''
    Returns list of paths to ROOT files corresponding to a given job
    '''

    return []
# ---------------------------------
def _split_paths(l_path):
    '''
    Takes list of paths to ROOT files
    Splits them into categories and returns a dictionary:

    category : [path_1, path_2, ...]
    '''
    npath = len(l_path)
    log.info(f'Splitting {npath} paths into categories')

    return {'none' : []}
# ---------------------------------
def _link_paths(kind, l_path):
    '''
    Makes symbolic links of list of paths of a specific kind
    '''
    npath = len(l_path)
    log.info(f'Linking {npath} {kind} paths')
# ---------------------------------
def _merge_paths(kind, l_path):
    '''
    Merge ROOT files of a specific kind
    '''
    npath = len(l_path)
    log.info(f'Merging {npath} {kind} paths')
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    l_path = _get_paths()
    d_path = _split_paths(l_path)
    for kind, l_path in d_path.items():
        _link_paths(kind, l_path)
        _merge_paths(kind, l_path)
# ---------------------------------
if __name__ == '__main__':
    main()
