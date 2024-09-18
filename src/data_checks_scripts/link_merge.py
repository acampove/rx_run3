#!/usr/bin/python3
'''
Script used to link ntuples properly and merge them
'''

import re
import os
import glob
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
    job     : str
    inp_dir : str = '/publicfs/lhcb/user/campoverde/Data/RK/'
    rgx     : str = r'(dt|mc)_(\d{4}).*ftuple_Hlt2RD_(.*)\.root'
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
    path_wc = f'{Data.inp_dir}/.run3/{Data.job}/*.root'
    l_path  = glob.glob(path_wc)

    npath   = len(l_path)
    if npath == 0:
        log.error(f'No file found in: {path_wc}')
        raise FileNotFoundError

    log.info(f'Found {npath} paths')

    return l_path
# ---------------------------------
def _split_paths(l_path):
    '''
    Takes list of paths to ROOT files
    Splits them into categories and returns a dictionary:

    category : [path_1, path_2, ...]
    '''
    npath = len(l_path)
    log.info(f'Splitting {npath} paths into categories')


    d_info_path = {}
    for path in l_path:
        info = _info_from_path(path)
        if info not in d_info_path:
            d_info_path[info] = []

        d_info_path[info].append(path)

    return d_info_path
# ---------------------------------
def _info_from_path(path):
    '''
    Will pick a path to a ROOT file
    Will return tuple with information associated to file
    This is needed to name output file and directories
    '''

    name = os.path.basename(path)
    mtc  = re.match(Data.rgx, name)
    if not mtc:
        log.error(f'Cannot find kind in {name} using {Data.rgx}')
        raise ValueError

    try:
        [dtmc, year, decay] = mtc.groups()
    except ValueError as exc:
        log.error(f'Expected three elements in: {mtc.groups()}')
        raise ValueError from exc

    if 'MuMu' in decay:
        chan = 'mm'
    elif 'EE' in decay:
        chan = 'ee'
    else:
        log.error(f'Cannot find channel in {decay}')
        raise ValueError

    dtmc = 'data' if dtmc == 'dt' else dtmc

    kind = _kind_from_decay(decay) 

    return dtmc, chan, kind, year
# ---------------------------------
def _kind_from_decay(decay):
    '''
    Will take string symbolizing decay
    Will return kind of sample associated, e.g. analysis, calibration, same sign...
    '''

    # TODO: This needs a config file 
    if decay in ['B0ToKpPimEE', 'B0ToKpPimMuMu']:
        return 'ana_cut_bd'

    if decay in ['BuToKpEE', 'BuToKpMuMu']:
        return 'ana_cut_bp'

    if decay in ['LbToLEE_LL', 'LbToLMuMu_LL']:
        return 'ana_cut_lb'


    if decay in ['B0ToKpPimEE_MVA', 'B0ToKpPimMuMu_MVA']:
        return 'ana_mva_bd'

    if decay in ['BuToKpEE_MVA', 'BuToKpMuMu_MVA']:
        return 'ana_mva_bp'

    if decay in ['LbToLEE_LL_MVA', 'LbToLMuMu_LL_MVA']:
        return 'ana_mva_lb'

    log.error(f'Unrecognized decay: {decay}')
    raise ValueError
# ---------------------------------
def _link_paths(kind, l_path):
    '''
    Makes symbolic links of list of paths of a specific kind
    '''
    npath = len(l_path)
    log.info(f'Linking {npath} paths {kind}')
# ---------------------------------
def _merge_paths(kind, l_path):
    '''
    Merge ROOT files of a specific kind
    '''
    npath = len(l_path)
    log.info(f'Merging {npath} paths {kind}')
    log.info('')
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
