'''
Script used to download filtered ntuples
from the grid
'''

#!/usr/bin/env python3

import os
import math
import random
import argparse

from concurrent.futures  import ThreadPoolExecutor
from dataclasses         import dataclass
from XRootD              import client              as clt
from XRootD.client.flags import DirListFlags
from log_store           import log_store

import tqdm

log = log_store.add_logger('data_checks:run3_download_ntuples')
# --------------------------------------------------
@dataclass
class Data:
    '''
    Class used to store attributes to be shared in script
    '''
    # pylint: disable = too-many-instance-attributes
    # Need this class to store data

    eos_dir : str        = '/eos/lhcb/grid/user/lhcb/user/a/acampove'
    server  : str        = 'root://eoslhcb.cern.ch/'
    nthread : int        = 1
    des_dir : str | None = None
    job_dir : str | None = None
    test    : int | None = None
    ran_pfn : int | None = None
    nfile   : int | None = None
    log_lvl : int | None = None

    eos_clt              = clt.FileSystem(server)
# --------------------------------------------------
def _download(pfn=None):
    if Data.test == 1:
        return

    file_name        = os.path.basename(pfn)
    out_path         = f'{Data.des_dir}/{file_name}'
    if os.path.isfile(out_path):
        log.debug('Skipping downloaded file')
        return

    log.debug(f'Downloading: {pfn} -> {out_path}')

    xrd_client = clt.FileSystem(pfn)
    status, _  = xrd_client.copy(pfn, out_path)
    _check_status(status, '_download')
# --------------------------------------------------
def _download_group(l_pfn=None, pbar=None):
    for pfn in l_pfn:
        _download(pfn)
        pbar.update(1)
# --------------------------------------------------
def _check_status(status, kind):
    if status.ok:
        log.debug(f'Successfully ran: {kind}')
    else:
        log.error(f'Failed to run {kind}: {status.message}')
        raise
# --------------------------------------------------
def _get_pfn_sublist(l_pfn):
    '''
    Return (optionally random) subset of LFNs out of l_lfn
    '''
    if Data.nfile < 0:
        log.debug('Negative number of files specified, will download everything')
        return l_pfn

    if Data.ran_pfn:
        log.debug('Downloading random {Data.nfile} files')
        l_pfn = random.sample(l_pfn, Data.nfile)
    else:
        log.debug('Downloading first {Data.nfile} files')
        l_pfn = l_pfn[:Data.nfile]

    return l_pfn
# --------------------------------------------------
def _get_pfns():
    file_dir = f'{Data.eos_dir}/{Data.job_dir}'
    status, listing = Data.eos_clt.dirlist(file_dir, DirListFlags.STAT)
    _check_status(status, '_get_pfns')

    l_pfn = [f'{Data.server}/{file_dir}/{entry.name}' for entry in listing ]
    l_pfn = _get_pfn_sublist(l_pfn)

    npfn = len(l_pfn)
    if npfn == 0:
        log.error(f'Found no PFNs in {file_dir}')
        raise

    log.info(f'Found {npfn} PFNs in {file_dir}')

    return l_pfn
# --------------------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Script used to download ntuples from EOS')
    parser.add_argument('-j', '--jobn' , type=str, help='Job name, used to find directory, e.g. flt_001', required=True)
    parser.add_argument('-n', '--nfile', type=int, help='Number of files to download', default=-1)
    parser.add_argument('-d', '--dest' , type=str, help='Output directory, will be CWD/job_name if not pased')
    parser.add_argument('-t', '--test' , type=int, help='Runs a test run with 1, default=0', default=0, choices=[0, 1])
    parser.add_argument('-l', '--log'  , type=int, help='Log level, default 20', choices=[10, 20, 30, 40], default=20)
    parser.add_argument('-r', '--ran'  , type=int, help='When picking a subset of files, with -n, pick them randomly (1) or the first files (0 default)', choices=[0, 1], default=0)
    parser.add_argument('-m', '--mth'  , type=int, help=f'Number of threads to use for downloading, default {Data.nthread}', default=Data.nthread)
    args = parser.parse_args()

    Data.job_dir = args.jobn
    Data.des_dir = args.dest
    Data.nfile   = args.nfile
    Data.test    = args.test
    Data.log_lvl = args.log
    Data.ran_pfn = args.ran
    Data.nthread = args.mth
# --------------------------------------------------
def _set_destination():
    if Data.des_dir is not None:
        log.debug(f'Destination directory already specified as {Data.des_dir}, not setting it')
        return

    Data.des_dir = f'{os.getcwd()}/{Data.job_dir}'
    log.info(f'Destination directory not found, using {Data.des_dir}')
# --------------------------------------------------
def _split_pfns(l_pfn):
    '''
    Takes a list of strings and splits it into many lists 
    to be distributed among nthread threads
    '''

    npfn         = len(l_pfn)
    thread_size  = math.floor(npfn / Data.nthread)

    log.debug(f'Splitting into {Data.nthread} threads with max size {thread_size} ')

    l_l_pfn = [ l_pfn[i_pfn : i_pfn + thread_size ] for i_pfn in range(0, npfn, thread_size)]

    return l_l_pfn
# --------------------------------------------------
def _initialize():
    log.setLevel(Data.log_lvl)

    _set_destination()
    os.makedirs(Data.des_dir, exist_ok=True)
# --------------------------------------------------
def main():
    '''
    start here
    '''
    _get_args()
    _initialize()

    l_pfn   = _get_pfns()
    l_l_pfn = _split_pfns(l_pfn)
    with ThreadPoolExecutor(max_workers=Data.nthread) as executor:
        for l_pfn in l_l_pfn:
            pbar = tqdm.tqdm(total=len(l_pfn))
            executor.submit(_download_group, l_pfn, pbar)
# --------------------------------------------------


if __name__ == '__main__':
    main()
