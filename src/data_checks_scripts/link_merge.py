#!/usr/bin/python3
'''
Script used to link ntuples properly and merge them
'''

import re
import os
import glob
import argparse

from dataclasses import dataclass

import tqdm

from ROOT      import TFileMerger
from log_store import log_store

log = log_store.add_logger('rx:data_checks:link_merge')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    job     : str
    dry     : int
    Max     : int
    ver     : str
    inp_dir : str = '/publicfs/lhcb/user/campoverde/Data/RK'
    rgx     : str = r'(dt|mc)_(\d{4}).*ftuple_Hlt2RD_(.*)\.root'
# ---------------------------------
def _get_args():
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-j', '--job', type=str, help='Job name, e.g. flt_001', required=True)
    parser.add_argument('-d', '--dry', type=int, help='Dry run if 1 (default)', choices=[0, 1], default=1)
    parser.add_argument('-l', '--lvl', type=int, help='log level', choices=[10, 20, 30], default=20)
    parser.add_argument('-m', '--max', type=int, help='Maximum number of paths, for test runs', default=-1)
    parser.add_argument('-v', '--ver', type=str, help='Version used to name outputs', required=True)
    args = parser.parse_args()

    Data.job = args.job
    Data.dry = args.dry
    Data.Max = args.max
    Data.ver = args.ver

    log.setLevel(args.lvl)
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

    d_info_path = _truncate_paths(d_info_path) 

    return d_info_path
# ---------------------------------
def _truncate_paths(d_path):
    '''
    Will limit the number of paths in the values if Data.Max is larger than zero
    '''

    if Data.Max < 0:
        return d_path

    log.warning(f'Truncating to {Data.Max} paths')

    d_path_trunc = { key : val[:Data.Max] for key, val in d_path.items() }

    return d_path_trunc
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
def _link_paths(info, l_path):
    '''
    Makes symbolic links of list of paths of a specific kind
    info is a tuple with = (sample, channel, kind, year) information
    Will return directory where linked files are
    '''
    npath = len(l_path)
    log.info(f'Linking {npath} paths {info}')

    sam, chan, kind, year = info

    target_dir  = f'{Data.inp_dir}/{sam}_{chan}_{kind}/{Data.ver}/{year}'
    os.makedirs(target_dir, exist_ok=True)
    log.debug(f'Linking to: {target_dir}')

    for source_path in tqdm.tqdm(l_path, ascii=' -'):
        name = os.path.basename(source_path)
        target_path = f'{target_dir}/{name}'

        log.debug(f'{source_path:<50}{"->":10}{target_path:<50}')
        if not Data.dry:
            _do_link_paths(src=source_path, tgt=target_path)

    return target_dir
# ---------------------------------
def _do_link_paths(src : str | None = None, tgt : str | None = None):
    '''
    Will check if target link exists, will delete it if it does
    Will make link
    '''

    if os.path.exists(tgt):
        os.unlink(tgt)

    os.symlink(src, tgt)
# ---------------------------------
def _merge_paths(target, l_path):
    '''
    Merge ROOT files of a specific kind
    '''
    npath = len(l_path)
    log.info(f'Merging {npath} paths {target}')
    log.info('')

    fm = TFileMerger(isLocal=True)
    for path in tqdm.tqdm(l_path, ascii=' -'):
        fm.AddFile(path, cpProgress=False)

    fm.OutputFile(target)
    status = fm.Merge()

    if not status:
        log.error(f'Could not create merged file: {target}')
        raise
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    l_path = _get_paths()
    d_path = _split_paths(l_path)
    for kind, l_path in d_path.items():
        target_dir = _link_paths(kind, l_path)
        target     = f'{target_dir}.root'
        _merge_paths(target, l_path)
# ---------------------------------
if __name__ == '__main__':
    main()
