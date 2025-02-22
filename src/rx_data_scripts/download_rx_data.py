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
import yaml

from XRootD                 import client   as clt
from dmu.logging.log_store  import LogStore

from rx_data                import utilities as ut

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

    d_trig  : dict[str,int]
    vers    : str
    kind    : str
    nfile   : int
    log_lvl : int
    dst_dir : Union[str, None]
    eos_dir : str
    drun    : bool
    ran_pfn : bool
    force   : bool
    trg_path: str

    pfn_preffix = 'root://x509up_u1000@eoslhcb.cern.ch//eos/lhcb/grid/user'
    nthread     = 1
# --------------------------------------------------
def _download(pfn : str) -> None:
    file_name        = os.path.basename(pfn)
    out_path         = f'{Data.dst_dir}/{Data.vers}/{file_name}'
    if os.path.isfile(out_path):
        log.debug(f'Skipping downloaded file: {pfn}')
        return

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
def _has_good_trigger(pfn : str) -> bool:
    _, trigger = ut.info_from_path(pfn)

    is_good = trigger in Data.d_trig

    return is_good
# --------------------------------------------------
def _get_pfns() -> list[str]:
    json_wc = files('rx_data_lfns').joinpath(f'{Data.kind}/{Data.vers}/*.json')
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

    nold  = len(l_pfn)
    l_pfn = [ pfn for pfn in l_pfn if _has_good_trigger(pfn) ]
    nnew  = len(l_pfn)

    log.info(f'Filtering PFNs by trigger: {nold} -> {nnew}')

    return l_pfn
# --------------------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Script used to download ntuples from EOS')
    parser.add_argument('-t', '--trig' , type=str, help='Path to YAML file with list of triggers', required=True)
    parser.add_argument('-v', '--vers' , type=str, help='Version of LFNs'                        , required=True)
    parser.add_argument('-k', '--kind' , type=str, help='Type of production'                     , choices=['rx', 'lbpkmumu'], required=True)
    parser.add_argument('-n', '--nfile', type=int, help='Number of files to download', default=-1)
    parser.add_argument('-p', '--dest' , type=str, help='Destination directory will override whatever is in DOWNLOAD_NTUPPATH')
    parser.add_argument('-l', '--log'  , type=int, help='Log level, default 20', choices=[10, 20, 30, 40], default=20)
    parser.add_argument('-m', '--mth'  , type=int, help=f'Number of threads to use for downloading, default {Data.nthread}', default=Data.nthread)
    parser.add_argument('-r', '--ran'  ,           help='When picking a subset of files, with -n, pick them randomly', action='store_true')
    parser.add_argument('-d', '--dryr' ,           help='If used, it will skip downloads, but do everything else'    , action='store_true')
    parser.add_argument('-f', '--force',           help='If used, it will download even if output already exists'    , action='store_true')

    args = parser.parse_args()

    Data.trg_path= args.trig
    Data.vers    = args.vers
    Data.kind    = args.kind
    Data.nfile   = args.nfile
    Data.dst_dir = args.dest
    Data.log_lvl = args.log
    Data.nthread = args.mth
    Data.ran_pfn = args.ran
    Data.drun    = args.dryr
    Data.force   = args.force
# --------------------------------------------------
def _split_pfns(l_pfn : list[str]) -> list[list[str]]:
    '''
    Takes a list of strings and splits it into many lists
    to be distributed among nthread threads
    '''

    npfn         = len(l_pfn)
    thread_size  = math.floor(npfn / Data.nthread)

    l_l_pfn = [ l_pfn[i_pfn : i_pfn + thread_size ] for i_pfn in range(0, npfn, thread_size)]

    log.debug(30 * '-')
    log.debug(f'{"Thread":<10}{"PFNs":<20}')
    log.debug(30 * '-')
    for i_thread, l_pfn_thread in enumerate(l_l_pfn):
        npfn = len(l_pfn_thread)
        log.debug(f'{i_thread:<10}{npfn:<20}')

    return l_l_pfn
# --------------------------------------------------
def _initialize():
    LogStore.set_level('rx_data:download_rx_data', Data.log_lvl)

    if Data.dst_dir is None:
        if 'DOWNLOAD_NTUPPATH' not in os.environ:
            raise ValueError('DOWNLOAD_NTUPPATH not set and -d option not pased')

        Data.dst_dir = os.environ['DOWNLOAD_NTUPPATH']

    _make_out_dir()
    with open(Data.trg_path, encoding='utf-8') as ifile:
        Data.d_trig = yaml.safe_load(ifile)
# --------------------------------------------------
def _make_out_dir() -> None:
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
def _cleanup_pfns(l_pfn : list[str]) -> list[str]:
    '''
    Takes list of PFNs to download and:

    1. Make list of file names to download.
    2. Make list of downloaded file names.
    3. If there are donwloaded files that were not meant to be downloaded, ask user to delete them and delete them.
    4. Remove already downloaded PFNs from input list.
    5. Return list of not downloaded PFNs
    '''
    s_name_to_download = { os.path.basename(pfn) for pfn in l_pfn }
    wc_path_downloaded = f'{Data.dst_dir}/{Data.vers}/*.root'
    l_path_downloaded  = glob.glob(wc_path_downloaded)
    if len(l_path_downloaded) == 0:
        log.info(f'No downloaded files found in {wc_path_downloaded}, check for superfluous paths skipped')
        return l_pfn

    s_name_downloaded  = {os.path.basename(path) for path in l_path_downloaded}
    s_name_superfluous = s_name_downloaded - s_name_to_download

    nsuperfluous = len(s_name_superfluous)
    if nsuperfluous != 0:
        log.info(f'Found {nsuperfluous} superfluous files in {wc_path_downloaded}')
        _delete_superfluous_files(s_name_superfluous)

    return _get_pfns_to_download(s_name_downloaded, l_pfn)
# --------------------------------------------------
def _get_pfns_to_download(s_name_downloaded : set[str], l_pfn : list[str]) -> list[str]:
    '''
    Takes names of files already downloaded, list of PFNs to download.
    Returns list of PFNs to download that have not been downloaded.
    '''

    nold = len(l_pfn)

    l_pfn_to_download = []
    for pfn in l_pfn:
        name_to_download = os.path.basename(pfn)
        if name_to_download in s_name_downloaded:
            continue

        l_pfn_to_download.append(pfn)

    nnew = len(l_pfn_to_download)

    log.warning(f'Will download {nnew} out of {nold} requested PFNs')

    return l_pfn_to_download
# --------------------------------------------------
def _delete_superfluous_files(s_name : set[str]) -> None:
    '''
    Takes set of names of files that need to be deleted, because they do not belong to list of files to download
    Asks user if these files should be deleted, after showing the list.
    Deletes the files
    '''
    nfile = len(s_name)
    log.warning(f'Found {nfile} superfluous files')

    for name in sorted(s_name):
        log.info(name)

    dec = input('Delete superfluous files? [y/n]')
    if dec not in ['y', 'n']:
        raise ValueError('Choose between [y,n]')

    if dec == 'n':
        log.debug('Not deleting superfluous files')
        return

    log.info('Deleting files:')
    for name in tqdm.tqdm(s_name, ascii=' -'):
        file_path = f'{Data.dst_dir}/{Data.vers}/{name}'
        if not os.path.isfile(file_path):
            raise ValueError(f'Cannot delete missing file: {file_path}')

        log.debug(file_path)
        if not Data.drun:
            os.remove(file_path)
# --------------------------------------------------
def main():
    '''
    start here
    '''
    _get_args()
    _initialize()

    l_pfn   = _get_pfns()
    l_pfn   = _cleanup_pfns(l_pfn)

    if len(l_pfn) == 0:
        return

    l_l_pfn = _split_pfns(l_pfn)
    ngroup  = len(l_l_pfn)

    log.info(f'Downloading {ngroup} groups with {Data.nthread} threads')
    with ThreadPoolExecutor(max_workers=Data.nthread) as executor:
        l_future = []
        for l_pfn in l_l_pfn:
            pbar = tqdm.tqdm(total=len(l_pfn))
            future = executor.submit(_download_group, l_pfn, pbar)
            l_future.append(future)

        for future in l_future:
            if future.exception():
                print(future.exception())
# --------------------------------------------------
if __name__ == '__main__':
    main()
