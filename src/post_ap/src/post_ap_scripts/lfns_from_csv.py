'''
Script that will make a list of dirac job IDs into a
text file with LFNs
'''
import os
import glob
import json
import argparse
from typing              import Union
from importlib.resources import files

import tqdm
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('post_ap:lfns_from_csv')
# ----------------------------
class Data:
    '''
    Data storing shared attributes
    '''
    fpath    : Union[str,None]
    version  : str
    dry_run  : bool
    eos_dir  = '/eos/lhcb/grid/user'
    lfn_dir  = '/lhcb/user/a/acampove'
    l_id     : list[str]
# ----------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Will use apd to save a list of paths to ROOT files in EOS')
    parser.add_argument('-f', '--fpath'   , type=str, help='Path to CSV file with job IDs')
    parser.add_argument('-v', '--version' , type=str, help='Version of production, needed to retrieve CSV file from rx_data')
    parser.add_argument('-l', '--loglevel', type=int, help='Controls logging level', choices=[10, 20, 30], default=20)
    parser.add_argument('-d', '--dry_run' ,           help='Dry run, if used will not search for ROOT paths', action='store_true')
    args = parser.parse_args()

    Data.version = args.version
    Data.fpath   = args.fpath
    Data.dry_run = args.dry_run
    LogStore.set_level('post_ap:lfns_from_csv', args.loglevel)
# ----------------------------
def _fpath_from_rxdata() -> list[str]:
    if Data.fpath is not None:
        log.info(f'Using user provided file with IDs: {Data.fpath}')
        if not os.path.isfile(Data.fpath):
            raise FileNotFoundError(f'Cannot find: {Data.fpath}')
        l_id_path = [Data.fpath]
    else:
        log.info(f'Using lists of IDs from {Data.version} in rx_data')
        path_wc   = files('rx_data_lfns').joinpath(f'{Data.version}/*.txt')
        path_wc   = str(path_wc)
        l_id_path = glob.glob(path_wc)

    nfiles = len(l_id_path)

    if nfiles == 0:
        raise FileNotFoundError(f'Missing files: {path_wc}')

    return l_id_path
# ----------------------------
def _get_jobids() -> list[str]:
    l_id_path = _fpath_from_rxdata()
    l_id = []
    for id_path in l_id_path:
        with open(id_path, encoding='utf-8') as ifile:
            l_id += ifile.read().splitlines()

    nid = len(l_id)
    log.info(f'Found {nid} job IDs')

    return l_id
# ----------------------------
def _get_lfns() -> list[str]:
    l_wc = [ f'{Data.eos_dir}{Data.lfn_dir}/*/{jobid[:-3]}/{jobid}/' for jobid in Data.l_id ]

    if Data.dry_run:
        log.warning('Running dry run, will not search for files')
        return []

    l_lfn = []
    for wc in tqdm.tqdm(l_wc, ascii=' -'):
        l_path  = glob.glob(f'{wc}/*.root')
        l_lfn  += [ lfn.replace(Data.eos_dir, '') for lfn in l_path ]

    nlfn = len(l_lfn)
    if nlfn == 0:
        raise FileNotFoundError('No LFN was found')

    log.info(f'Found {nlfn} LFNs')

    return l_lfn
# ----------------------------
def _initialize() -> None:
    Data.l_id = _get_jobids()

    if not Data.dry_run and not os.path.isdir(Data.eos_dir):
        raise FileNotFoundError(f'Missing grid directory: {Data.eos_dir}')

    log.debug(f'Looking into: {Data.eos_dir}')
# ----------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    l_lfn = _get_lfns()
    with open('lfns.json', 'w', encoding='utf-8') as ofile:
        json.dump(l_lfn, ofile, indent=4)

    log.info('Saved LFNs')
# ----------------------------
if __name__ == '__main__':
    main()
