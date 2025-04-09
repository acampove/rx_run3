'''
Script used to copy ntuples from mounted filesystem
'''
import os
import glob
import shutil
import argparse
from importlib.resources import files

import tqdm
import yaml
from dmu.generic.version_management import get_last_version
from dmu.logging.log_store          import LogStore
from rx_data                        import utilities as ut

log = LogStore.add_logger('rx_data:copy_samples')
# -----------------------------------------
class Data:
    '''
    Class holding attributes meant to be shared
    '''
    kind  : str
    conf  : str
    dry   : bool

    d_conf  : dict
    d_data  : dict
    out_dir : str
    l_source: list[str]

    vers    = None
    l_kind  = [
            'all',
            'main',
            'mva',
            'hop',
            'swp_jpsi_misid',
            'swp_cascade',
            'ecalo_bias',
            'brem_track_2']
# -----------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to copy files from remote server to laptop')
    parser.add_argument('-k', '--kind', type=str, help='Type of files', choices=Data.l_kind, required=True)
    parser.add_argument('-c', '--conf', type=str, help='Name of YAML config file, e.g. rk', required=True)
    parser.add_argument('-l', '--logl', type=int, help='Logger level', choices=[10, 20, 30], default=20)
    parser.add_argument('-v', '--vers', type=str, help='Version of files, only makes sense if kind is not "all"')
    parser.add_argument('-d', '--dry' ,           help='If used, will do not copy files', action='store_true')
    args = parser.parse_args()

    Data.kind = args.kind
    Data.conf = args.conf
    Data.vers = args.vers
    Data.dry  = args.dry

    LogStore.set_level('rx_data:copy_samples', args.logl)
# -----------------------------------------
def _is_right_trigger(path : str) -> bool:
    l_trigger  = Data.d_conf['triggers']
    _, trigger = ut.info_from_path(path)

    return trigger in l_trigger
# -----------------------------------------
def _get_source_paths() -> list[str]:
    d_samp   = Data.d_conf['samples']
    l_source = []
    log.info(70 * '-')
    log.info(f'{"Sample":<20}{"Identifier":<30}{"Paths":<20}')
    log.info(70 * '-')
    for sample, l_identifier in d_samp.items():
        for identifier in l_identifier:
            identifier    = str(identifier)
            l_source_samp = [ source for source in Data.l_source if identifier in source and _is_right_trigger(source) ]
            npath = len(l_source_samp)
            log.info(f'{sample:<20}{identifier:<30}{npath:<20}')
            l_source += l_source_samp

    log.info(70 * '-')

    nsource = len(l_source)
    if nsource == 0:
        raise ValueError('Will not copy any file')

    log.info(f'Will copy {nsource} files')
    for source in l_source:
        log.debug(source)

    return l_source
# -----------------------------------------
def _get_version(kind : str) -> str:
    if Data.vers is not None:
        return Data.vers

    inp_dir = Data.d_conf['inp_dir']
    vers    = get_last_version(dir_path = f'{inp_dir}/{kind}', version_only=True)

    return vers
# -----------------------------------------
def _initialize_kind(kind : str):
    if Data.vers is not None and Data.kind == 'all':
        raise ValueError(f'Specified version {Data.vers} for kind {Data.kind}')

    vers    = _get_version(kind)
    inp_dir = Data.d_conf['inp_dir']
    inp_dir = f'{inp_dir}/{kind}/{vers}'
    path_wc = f'{inp_dir}/*.root'
    l_path  = glob.glob(path_wc)

    out_dir = Data.d_conf['out_dir']
    Data.out_dir = f'{out_dir}/{kind}/{vers}'
    os.makedirs(Data.out_dir, exist_ok=True)


    log.info(f'Source: {inp_dir}')
    log.info(f'Target: {Data.out_dir}')

    nsource = len(l_path)
    if nsource == 0:
        raise ValueError(f'No files found in: {path_wc}')

    log.info(f'Found {nsource} files')

    Data.l_source = l_path
# -----------------------------------------
def _initialize() -> None:
    cfg_path = files('rx_data_data').joinpath(f'copy_files/{Data.conf}.yaml')
    with open(cfg_path, encoding='utf-8') as ifile:
        Data.d_conf = yaml.safe_load(ifile)
# -----------------------------------------
def _copy_sample(source : str) -> int:
    fname = os.path.basename(source)
    target= f'{Data.out_dir}/{fname}'

    if os.path.isfile(target):
        log.debug(f'Target found, skipping: {target}')
        return 0

    if not Data.dry:
        log.debug('')
        log.debug(source)
        log.debug('--->')
        log.debug(target)
        log.debug('')
        shutil.copy(source, target)
        return 1

    return 0
# -----------------------------------------
def _download_kind(kind : str):
    if kind == 'all':
        return

    log.info(f'Copying files for kind {kind}')
    _initialize_kind(kind)

    l_path = _get_source_paths()
    ncopied= 0
    for path in tqdm.tqdm(l_path, ascii=' -'):
        ncopied += _copy_sample(path)

    log.info(f'Copied {ncopied} files for kind {kind}')
# -----------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    _initialize()

    if Data.kind == 'all':
        l_kind = Data.l_kind
    else:
        l_kind = [Data.kind]

    for kind in l_kind:
        _download_kind(kind)
# -----------------------------------------
if __name__ == '__main__':
    main()
