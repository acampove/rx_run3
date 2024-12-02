'''
Script used to download filtered ntuples
from the grid
'''

import os
import math
import random
import argparse

from concurrent.futures     import ThreadPoolExecutor
from dataclasses            import dataclass
from XRootD                 import client              as clt
from XRootD.client.flags    import DirListFlags
from dmu.logging.log_store  import LogStore

import tqdm

log = LogStore.add_logger('post_ap:run3_download_ntuples')
# --------------------------------------------------
@dataclass
class Data:
    '''
    Class used to store attributes to be shared in script
    '''
    # pylint: disable = too-many-instance-attributes
    # Need this class to store data

    job_dir : str
    nfile   : int
    log_lvl : int
    dst_dir : str

    ran_pfn : int
    test    = False
    lxname  = os.environ['LXNAME']
    eos_dir = f'/eos/lhcb/grid/user/lhcb/user/{lxname[0]}/{lxname}'
    server  = 'root://eoslhcb.cern.ch/'
    eos_clt = clt.FileSystem(server)
    nthread = 1
# --------------------------------------------------
def _download(pfn : str) -> None:
    file_name        = os.path.basename(pfn)
    out_path         = f'{Data.dst_dir}/{Data.job_dir}/{file_name}'
    if os.path.isfile(out_path):
        log.debug('Skipping downloaded file')
        return

    log.debug(f'Downloading: {pfn} -> {out_path}')
    if Data.test:
        return

    xrd_client = clt.FileSystem(pfn)
    status, _  = xrd_client.copy(pfn, out_path)
    _check_status(status, '_download')
# --------------------------------------------------
def _download_group(l_pfn : list[str], pbar : tqdm.std.tqdm):
    for pfn in l_pfn:
        _download(pfn)
        pbar.update(1)
# --------------------------------------------------
def _check_status(status, kind):
    if status.ok:
        log.debug(f'Successfully ran: {kind}')
    else:
        raise ValueError(f'Failed to run {kind}: {status.message}')
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
        raise ValueError(f'Found no PFNs in {file_dir}')

    log.info(f'Found {npfn} PFNs in {file_dir}')

    return l_pfn
# --------------------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Script used to download ntuples from EOS')
    parser.add_argument('-j', '--jobn' , type=str, help='Job name, used to find directory, e.g. flt_001', required=True)
    parser.add_argument('-n', '--nfile', type=int, help='Number of files to download', default=-1)
    parser.add_argument('-d', '--dest' , type=str, help='Destination directory will override whatever is in DOWNLOAD_NTUPPATH')
    parser.add_argument('-t', '--test' , help='Runs a test run', action='store_true')
    parser.add_argument('-l', '--log'  , type=int, help='Log level, default 20', choices=[10, 20, 30, 40], default=20)
    parser.add_argument('-r', '--ran'  , type=int, help='When picking a subset of files, with -n, pick them randomly (1) or the first files (0 default)', choices=[0, 1], default=0)
    parser.add_argument('-m', '--mth'  , type=int, help=f'Number of threads to use for downloading, default {Data.nthread}', default=Data.nthread)
    args = parser.parse_args()

    Data.job_dir = args.jobn
    Data.dst_dir = args.dest
    Data.nfile   = args.nfile
    Data.test    = args.test
    Data.log_lvl = args.log
    Data.ran_pfn = args.ran
    Data.nthread = args.mth
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

    if Data.dst_dir is None:
        if 'DOWNLOAD_NTUPPATH' not in os.environ:
            raise ValueError('DOWNLOAD_NTUPPATH not set and -d option not pased')

        Data.dst_dir = os.environ['DOWNLOAD_NTUPPATH']

    os.makedirs(f'{Data.dst_dir}/{Data.job_dir}', exist_ok=True)
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
