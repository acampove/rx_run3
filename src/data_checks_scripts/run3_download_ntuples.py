#!/usr/bin/env python3

from XRootD              import client as clt
from XRootD.client.flags import DirListFlags
from log_store           import log_store

import os
import tqdm
import random
import argparse

log = log_store.add_logger('data_checks:run3_download_ntuples')
# --------------------------------------------------
class data:
    server  = 'root://eoslhcb.cern.ch/'
    eos_clt = clt.FileSystem(server)
    eos_dir = '/eos/lhcb/grid/user/lhcb/user/a/acampove'
    job_dir = None
    des_dir = None
    test    = None
    log_lvl = None
    ran_pfn = None
    nfile   = None
# --------------------------------------------------
def _download(pfn=None):
    if data.test == 1:
        return

    file_name        = os.path.basename(pfn)
    out_path         = f'{data.des_dir}/{file_name}'
    if os.path.isfile(out_path):
        log.debug(f'Skipping downloaded file')
        return

    log.debug(f'Downloading: {pfn} -> {out_path}')

    xrd_client       = clt.FileSystem(pfn)
    status, response = xrd_client.copy(pfn, out_path)
    _check_status(status, '_download')
# --------------------------------------------------
def _check_status(status, kind):
    if status.ok:
        log.debug(f'Successfully ran: {kind}')
    else:
        log.error(f'Failed to run {kind}: {status.message}')
        raise
# --------------------------------------------------
def _get_pfn_sublist(l_pfn):
    if data.nfile < 0:
        log.debug('Negative number of files specified, will download everything')
        return l_pfn

    if data.ran_pfn:
        log.debug('Downloading random {data.nfile} files')
        l_pfn = random.sample(l_pfn, data.nfile)
    else:
        log.debug('Downloading first {data.nfile} files')
        l_pfn = l_pfn[:data.nfile] 

    return l_pfn
# --------------------------------------------------
def _get_pfns():
    file_dir = f'{data.eos_dir}/{data.job_dir}'
    status, listing = data.eos_clt.dirlist(file_dir, DirListFlags.STAT)
    _check_status(status, '_get_pfns')

    l_pfn = [f'{data.server}/{file_dir}/{entry.name}' for entry in listing ]
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
    parser.add_argument('-j', '--jobn' , type=str, help='Job name, used to find directory', required=True)
    parser.add_argument('-n', '--nfile', type=int, help='Number of files to download', default=-1, required=False)
    parser.add_argument('-d', '--dest' , type=str, help='Directory where files will be placed', required=True)
    parser.add_argument('-t', '--test' , type=int, help='Runs a test run with 1, default=0', default=0, choices=[0, 1])
    parser.add_argument('-l', '--log'  , type=int, help='Log level, default 20', choices=[10, 20, 30, 40], default=20)
    parser.add_argument('-r', '--ran'  , type=int, help='When picking a subset of files, with -n, \
                        pick them randomly (1) or the first files (0 default)', choices=[0, 1], default=0)
    args = parser.parse_args()

    data.job_dir = args.jobn
    data.des_dir = args.dest
    data.nfile   = args.nfile
    data.test    = args.test
    data.log_lvl = args.log
    data.ran_pfn = args.ran
# --------------------------------------------------
def _initialize():
    log.setLevel(data.log_lvl)
    os.makedirs(data.des_dir, exist_ok=True)
# --------------------------------------------------
def main():
    _get_args()
    _initialize()

    l_pfn = _get_pfns()
    for pfn in tqdm.tqdm(l_pfn, ascii=' -'):
        _download(pfn=pfn)
# --------------------------------------------------


if __name__ == '__main__':
    main()
