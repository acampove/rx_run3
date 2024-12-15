'''
Script used to download filtered ntuples
from the grid
'''

import os
import math
import json
import glob
import random
import argparse

from typing                 import Union
from importlib.resources    import files
from concurrent.futures     import ThreadPoolExecutor
from dataclasses            import dataclass

import tqdm

from XRootD                 import client   as clt
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:download_rx_data')

# pylint: disable=line-too-long
# --------------------------------------------------
@dataclass
class Data:
    '''
    Class used to store attributes to be shared in script
    '''
    # pylint: disable = too-many-instance-attributes
    # Need this class to store data

    vers    : str
    nfile   : int
    log_lvl : int
    dst_dir : Union[str, None]
    eos_dir : str
    drun    : bool
    ran_pfn : bool
    force   : bool

    pfn_preffix = 'root://x509up_u1000@eoslhcb.cern.ch//eos/lhcb/grid/user'
    nthread     = 1
# --------------------------------------------------
def _download(pfn : str) -> None:
    file_name        = os.path.basename(pfn)
    out_path         = f'{Data.dst_dir}/{Data.vers}/{file_name}'
    if os.path.isfile(out_path):
        log.debug(f'Skipping downloaded file: {pfn}')
        return

    log.debug(f'   {pfn}'     )
    log.debug(f'   {out_path}')
    if Data.drun:
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
def _get_pfn_subset(l_pfn : list[str]) -> list[str]:
    if not Data.ran_pfn:
        log.warning(f'Picking up a subset of the first {Data.nfile} ntuples')
        return l_pfn[:Data.nfile]

    log.warning(f'Picking up a random subset of {Data.nfile} ntuples')

    l_pfn = random.sample(l_pfn, Data.nfile)

    return l_pfn
# --------------------------------------------------
def _get_pfns() -> list[str]:
    json_wc = files('rx_data_lfns').joinpath(f'{Data.vers}/*.json')
    json_wc = str(json_wc)
    l_json  = glob.glob(json_wc)

    l_lfn   = []
    for json_path in l_json:
        with open(json_path, encoding='utf-8') as ifile:
            l_lfn += json.load(ifile)

    nlfn    = len(l_lfn)
    if nlfn == 0:
        raise ValueError(f'''
        -------------------------------------------------------------------
                         Found {nlfn} LFNs for version {Data.vers}, either:

                         1. You wrote the wrong version.
                         2. You forgot to run pip install --upgrade rx_data
        -------------------------------------------------------------------
                         ''')

    log.info(f'Found {nlfn} paths')
    l_pfn   = [ f'{Data.pfn_preffix}/{LFN}' for LFN in l_lfn ]

    if Data.nfile > 0:
        l_pfn = _get_pfn_subset(l_pfn)

    return l_pfn
# --------------------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Script used to download ntuples from EOS')
    parser.add_argument('-v', '--vers' , type=str, help='Version of LFNs'            , required=True)
    parser.add_argument('-n', '--nfile', type=int, help='Number of files to download', default=-1)
    parser.add_argument('-p', '--dest' , type=str, help='Destination directory will override whatever is in DOWNLOAD_NTUPPATH')
    parser.add_argument('-l', '--log'  , type=int, help='Log level, default 20', choices=[10, 20, 30, 40], default=20)
    parser.add_argument('-m', '--mth'  , type=int, help=f'Number of threads to use for downloading, default {Data.nthread}', default=Data.nthread)
    parser.add_argument('-r', '--ran'  ,           help='When picking a subset of files, with -n, pick them randomly', action='store_true')
    parser.add_argument('-d', '--dryr' ,           help='If used, it will skip downloads, but do everything else'    , action='store_true')
    parser.add_argument('-f', '--force',           help='If used, it will download even if output already exists'    , action='store_true')

    args = parser.parse_args()

    Data.vers    = args.vers
    Data.nfile   = args.nfile
    Data.dst_dir = args.dest
    Data.log_lvl = args.log
    Data.nthread = args.mth
    Data.ran_pfn = args.ran
    Data.drun    = args.dryr
    Data.force   = args.force
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
    LogStore.set_level('rx_data:download_rx_data', Data.log_lvl)

    if Data.dst_dir is None:
        if 'DOWNLOAD_NTUPPATH' not in os.environ:
            raise ValueError('DOWNLOAD_NTUPPATH not set and -d option not pased')

        Data.dst_dir = os.environ['DOWNLOAD_NTUPPATH']

    ntup_dir = f'{Data.dst_dir}/{Data.vers}'
    try:
        os.makedirs(ntup_dir, exist_ok=Data.force)
    except FileExistsError as exc:
        raise FileExistsError(f'''
        -------------------------------------------------------------------
        Version of ntuples {Data.vers} already found in {ntup_dir}, either:

        1. Partial download already happened and you are retrying, run with -f (--force) flag.
        2. You are not running the latest version of and you need to run:
                pip install --upgrade rx_data.
        -------------------------------------------------------------------
                              ''') from exc
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
