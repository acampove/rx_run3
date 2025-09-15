'''
Script used to copy ntuples from mounted filesystem
'''
import os
import shutil
import argparse
import multiprocessing
from pathlib             import Path

import tqdm
import numpy
from omegaconf                      import DictConfig
from dmu.generic.version_management import get_last_version
from dmu.logging.log_store          import LogStore
from dmu.generic                    import utilities as gut
from rx_data                        import utilities as ut

log = LogStore.add_logger('rx_data:copy_samples')
# -----------------------------------------
class Data:
    '''
    Class holding attributes meant to be shared
    '''
    kind    : str
    proj    : str
    dry     : bool
    nprc    : int
    conf  : DictConfig 
    d_data  : dict
    l_source: list[Path]

    out_dir : Path # Path to ana_dir/{kind}/{version}
    pfs_dir = Path(os.environ['PFS_ANADIR'])/'Data'
    ana_dir = Path(os.environ['ANADIR'    ])/'Data'
    vers    = None
    l_kind  = [
        'all',
        'main',
        'mva',
        'hop',
        'mass',
        'swp_jpsi_misid',
        'swp_cascade',
        'brem_track_2']
    copied_files : int   = 0
    copied_size  : float = 0
# -----------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to copy files from remote server to laptop')
    parser.add_argument('-k', '--kind', type=str, help='Type of files', choices=Data.l_kind, required=True)
    parser.add_argument('-p', '--proj', type=str, help='Name of YAML config file, e.g. rk', required=True, choices=['rk', 'rkst'])
    parser.add_argument('-l', '--logl', type=int, help='Logger level', choices=[5, 10, 20, 30], default=20)
    parser.add_argument('-n', '--nprc', type=int, help='Number of process to download with, with zero, will download all files at once', default=1)
    parser.add_argument('-v', '--vers', type=str, help='Version of files, only makes sense if kind is not "all"')
    parser.add_argument('-d', '--dry' ,           help='If used, will do not copy files', action='store_true')
    args = parser.parse_args()

    Data.kind = args.kind
    Data.proj = args.proj
    Data.conf = gut.load_conf(package='rx_data_data', fpath=f'copy_files/{args.proj}.yaml')
    Data.vers = args.vers
    Data.dry  = args.dry
    Data.nprc = args.nprc

    LogStore.set_level('rx_data:copy_samples', args.logl)
# -----------------------------------------
def _is_right_trigger(path : Path) -> bool:
    l_trigger  = Data.conf.triggers
    _, trigger = ut.info_from_path(path)

    return trigger in l_trigger
# -----------------------------------------
def _get_source_paths() -> list[Path]:
    d_samp   = Data.conf.samples
    l_source = []
    log.info(70 * '-')
    log.info(f'{"Sample":<20}{"Identifier":<30}{"Paths":<20}')
    log.info(70 * '-')
    for sample, l_identifier in d_samp.items():
        for identifier in l_identifier:
            identifier    = str(identifier)
            l_source_samp = [ source for source in Data.l_source if identifier in source.name and _is_right_trigger(source) ]

            npath     = len(l_source_samp)
            log.info(f'{sample:<20}{identifier:<30}{npath:<20}')
            l_source += l_source_samp

    log.info(70 * '-')

    nsource = len(l_source)
    if nsource == 0:
        raise ValueError('No files found')

    log.info(f'Found {nsource} files in source path')
    for source in l_source:
        log.verbose(source)

    return l_source
# -----------------------------------------
def _get_version(kind : str) -> str:
    '''
    Get latest or user defined version for given kind of friend trees 
    '''
    if Data.vers is not None:
        return Data.vers

    knd_dir = f'{Data.pfs_dir}/{Data.proj}/{kind}'
    vers    = get_last_version(dir_path = knd_dir, version_only=True)

    log.debug(f'Latest version {vers} found in {knd_dir}')

    return vers
# -----------------------------------------
def _initialize(kind : str):
    if Data.vers is not None and Data.kind == 'all':
        raise ValueError(f'Specified version {Data.vers} for kind {Data.kind}')

    vers    = _get_version(kind)
    inp_dir = Data.pfs_dir/f'{Data.proj}/{kind}/{vers}'
    l_path  = list(inp_dir.glob('*.root'))

    Data.out_dir = Data.ana_dir/f'{Data.proj}/{kind}/{vers}'
    Data.out_dir.mkdir(parents=True, exist_ok=True)

    log.info(f'Source: {inp_dir}')
    log.info(f'Target: {Data.out_dir}')

    nsource = len(l_path)
    if nsource == 0:
        raise ValueError(f'No files found in: {inp_dir}')

    log.info(f'Found {nsource} files')

    Data.l_source = l_path
# ----------------------
def _not_corrupted(source : Path, target : Path) -> bool:
    '''
    This function checks that copied path and original are identical

    Parameters
    -------------
    source: Path to file to be copied
    target: Path to copied file

    Returns
    -------------
    True if the file is not corrupted, False otherwise
    If it is corrupted, will remove target (local) file
    '''
    if source.stat().st_size != target.stat().st_size:
        log.warning('')
        log.warning(f'Files differ in size for: {target.name}')
        log.warning(f'Removing local {target.name}')
        target.unlink()
        return False

    size = source.stat().st_size
    log.verbose(f'Files agree in size ({size}) for: {source.name}')

    return True
# -----------------------------------------
def _copy_sample(source : Path) -> int:
    target= Data.out_dir/source.name

    if os.path.isfile(target) and _not_corrupted(source=source, target=target):
        log.verbose(f'Target found, skipping: {target}')
        log.verbose('')
        return 0

    Data.copied_files += 1
    Data.copied_size  += source.stat().st_size / 1_000_000_000
    log.debug('')
    log.debug(source)
    log.debug('--->')
    log.debug(target)
    log.debug('')

    if not Data.dry:
        shutil.copy(source, target)

    return 1
# -----------------------------------------
def _download_group(group : list[Path]) -> int:
    if len(group) == 1:
        ncopied = _copy_sample(source=group[0])
        return ncopied

    with multiprocessing.Pool() as pool:
        l_ncopied = pool.map(_copy_sample, group)
        ncopied   = sum(l_ncopied)

    return ncopied
# -----------------------------------------
def _group_paths(l_path : list[Path]) -> list[list[Path]]:
    '''
    Parameters
    --------------------
    l_path: List of paths of files to download

    Returns
    --------------------
    Group of lists of files to download. Each group will be downloaded separately
    '''
    if Data.nprc <  0:
        raise ValueError(f'Number of processes has to be larger or equal to 1, found: {Data.nprc}')

    if Data.nprc == 0:
        return [ l_path ]

    if Data.nprc == 1:
        l_group = [ [path] for path in l_path ]
        return l_group

    arr_path   = numpy.array(l_path, dtype=object)
    l_arr_path = numpy.array_split(arr_path, Data.nprc)
    l_l_path   = [ [ Path(obj) for obj in arr_path ] for arr_path in l_arr_path ]

    return l_l_path
# -----------------------------------------
def _download_kind(kind : str):
    if kind == 'all':
        return

    log.info(f'Copying files for kind {kind}')
    _initialize(kind)

    l_path = _get_source_paths()
    l_group= _group_paths(l_path)
    ncopied= 0
    for group in tqdm.tqdm(l_group, ascii=' -'):
        ncopied += _download_group(group)

    log.info(f'Copied {ncopied} files for kind {kind}')
# -----------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()

    if Data.kind == 'all':
        l_kind = Data.l_kind
    else:
        l_kind = [Data.kind]

    for kind in l_kind:
        _download_kind(kind)

    log.info(f'Copied {Data.copied_files} ({Data.copied_size:.2f} Gb) files in total')
# -----------------------------------------
if __name__ == '__main__':
    main()
