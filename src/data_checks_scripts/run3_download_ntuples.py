#!/usr/bin/env python3

from XRootD              import client as clt
from XRootD.client.flags import DirListFlags
from log_store           import log_store

import os
import random
import argparse

log=log_store.add_logger('data_checks:run3_download_ntuples')
#--------------------------------------------------
class data:
    server  = 'root://eoslhcb.cern.ch/'
    eos_clt = clt.FileSystem(server)
    eos_dir = '/eos/lhcb/grid/user/lhcb/user/a/acampove'
    job_dir = None
    des_dir = None
    test    = None
#--------------------------------------------------
def _download(pfn=None):
    log.debug(f'Downloading: {pfn}')
    if data.test == 1:
        return

    file_name        = os.path.basename(pfn)
    xrd_client       = clt.FileSystem(pfn)
    status, response = xrd_client.copy(pfn, f'{data.des_dir}/{file_name}')
    _check_status(status, '_download')
#--------------------------------------------------
def _check_status(status, kind):
    if status.ok:
        log.debug(f'Successfully ran: {kind}')
    else:
        log.debug(f'Failed to run {kind}: {status.message}')
        raise
#--------------------------------------------------
def _get_pfns():
    file_dir = f'{data.eos_dir}/{data.job_dir}'
    status, listing = data.eos_clt.dirlist(file_dir, DirListFlags.STAT)
    _check_status(status, '_get_pfns')

    l_pfn = [f'{data.server}/{file_dir}/{entry.name}' for entry in listing ]
    l_pfn = random.sample(l_pfn, data.nfile)

    npfn=len(l_pfn)
    if npfn == 0:
        log.error(f'Found no PFNs in {file_dir}')
        raise

    log.info(f'Found {npfn} PFNs in {file_dir}')

    return l_pfn
#--------------------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Script used to download ntuples from EOS')
    parser.add_argument('-j', '--jobn' , type=str, help='Job name, used to find directory', required=True)
    parser.add_argument('-n', '--nfile', type=int, help='Number of files to download, chosen randomly', required=True)
    parser.add_argument('-d', '--dest' , type=str, help='Directory where files will be placed', required=True)
    parser.add_argument('-t', '--test' , type=int, help='Runs a test run with 1, default=0', choices=[0, 1])
    parser.add_argument('-l', '--log'  , type=int, help='Log level', choices=[10, 20, 30, 40])
    args = parser.parse_args()

    data.job_dir = args.jobn
    data.des_dir = args.dest
    data.nfile   = args.nfile
    data.test    = args.test

    log.setLevel(args.log)
#--------------------------------------------------
def main():
    get_args()
    l_pfn = _get_pfns()
    for pfn in l_pfn:
        _download(pfn=pfn)
#--------------------------------------------------
if __name__ == '__main__':
    main()

