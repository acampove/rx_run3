'''
Script used to link ntuples properly and merge them
'''

# pylint: disable=line-too-long, import-error

import re
import os
import glob
import argparse

from typing                 import Union
from dataclasses            import dataclass
from dmu.rfile.rfprinter    import RFPrinter
from dmu.logging.log_store  import LogStore

import tqdm

from rx_data.path_splitter import PathSplitter

log   = LogStore.add_logger('rx_data:make_tree_structure')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    # pylint: disable = invalid-name

    max_files : int
    ver       : str
    dry       : bool
    inp_path  : str
    out_path  : str
# ---------------------------------
def _get_paths() -> list[str]:
    '''
    Returns list of paths to ROOT files corresponding to a given job
    '''
    path_wc = f'{Data.inp_path}/*.root'
    l_path  = glob.glob(path_wc)

    npath   = len(l_path)
    if npath == 0:
        log.error(f'No file found in: {path_wc}')
        raise FileNotFoundError

    log.info(f'Found {npath} paths')

    return l_path
# ---------------------------------
def _link_paths(sample : str, line : str, l_path : list[str]) -> Union[str, None]:
    '''
    Makes symbolic links of list of paths of a specific kind
    info is a tuple with = (sample, channel, kind, year) information
    Will return directory where linked files are
    '''
    npath = len(l_path)
    log.debug(f'Linking {npath} paths for {sample}/{line}')

    target_dir  = f'{Data.out_path}/{Data.ver}/post_ap/{sample}/{line}'
    os.makedirs(target_dir, exist_ok=True)

    log.debug(f'Linking to: {target_dir}')
    if Data.dry:
        log.warning('Dry run, not linking')
        return None

    for source_path in l_path:
        file_name   = os.path.basename(source_path)
        target_path = f'{target_dir}/{file_name}'

        log.debug(f'{source_path:<50}{"->":10}{target_path:<50}')
        _do_link_paths(src=source_path, tgt=target_path)

    return target_dir
# ---------------------------------
def _do_link_paths(src : str, tgt : str) -> None:
    '''
    Will check if target link exists, will delete it if it does
    Will make link
    '''
    if os.path.exists(tgt):
        os.unlink(tgt)

    os.symlink(src, tgt)
# ---------------------------------
def _save_summary(target_dir : str) -> None:
    '''
    Make text file with summary of file, e.g. 2024.root -> 2024.txt
    '''
    if Data.dry:
        return

    l_file_path = glob.glob(f'{target_dir}/*.root')
    if len(l_file_path) == 0:
        log.warning(f'No ROOT file found in {target_dir}')
        return

    target_file = l_file_path[0]

    prt = RFPrinter(path=target_file)
    prt.save(file_name='summary.txt', raise_on_fail=False)
# ---------------------------------
def _get_args() -> argparse.Namespace:
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Makes directory structure from ROOT files through symbolic links')
    parser.add_argument('-i', '--inp', type=str, help='Path to directory with ROOT files to link'        , required=True)
    parser.add_argument('-o', '--out', type=str, help='Path to directory where tree structure will start', required=True)
    parser.add_argument('-m', '--max', type=int, help='Maximum number of paths, for test runs'   , default=-1)
    parser.add_argument('-l', '--lvl', type=int, help='log level', choices=[10, 20, 30]          , default=20)
    parser.add_argument('-d', '--dry',           help='Dry run if 1', action='store_true')
    args = parser.parse_args()

    return args
# ---------------------------------
def _version_from_input() -> str:
    version = os.path.basename(Data.inp_path)
    if not re.match(r'v\d+', version):
        raise ValueError(f'Cannot extract version from: {version}')

    log.info(f'Using version {version}')

    return version
# ---------------------------------
def _initialize(args : argparse.Namespace) -> None:
    Data.dry       = args.dry
    Data.max_files = args.max
    Data.inp_path  = args.inp
    Data.out_path  = args.out

    LogStore.set_level('rx_data:make_tree_structure', args.lvl)
    LogStore.set_level('dmu:rfprinter', 30)

    Data.ver       = _version_from_input()
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    args = _get_args()
    _initialize(args)

    l_path = _get_paths()

    splt = PathSplitter(paths=l_path, max_files=Data.max_files)
    d_path = splt.split()

    for (sample, line), l_path in tqdm.tqdm(d_path.items(), ascii=' -'):

        target_dir = _link_paths(sample, line, l_path)
        if target_dir is None:
            continue

        _save_summary(target_dir)
# ---------------------------------
if __name__ == '__main__':
    main()
