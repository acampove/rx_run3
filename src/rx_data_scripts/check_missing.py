'''
Script used to check what samples are present in the main trees
but missing among the friend trees
'''
import os
import glob
from typing import Union

from dmu.generic.version_management import get_last_version
from dmu.logging.log_store          import LogStore

log = LogStore.add_logger('rx_data:check_missing')
# ---------------------------------
class Data:
    '''
    Data class
    '''
    data_dir : str
# ---------------------------------
def _initialize() -> None:
    if 'DATADIR' not in os.environ:
        raise FileNotFoundError('DATADIR environment variable not set')

    Data.data_dir = os.environ['DATADIR']
# ---------------------------------
def _version_from_path(path : str) -> Union[str,None]:
    try:
        version=get_last_version(dir_path=path, version_only=True)
    except ValueError:
        log.debug(f'No version found for {path}')
        return None

    return version
# ---------------------------------
def _paths_from_sample(path : str, version : str) -> set[str]:
    root_wc = f'{path}/{version}/*.root'
    l_path  = glob.glob(root_wc)

    if len(l_path) == 0:
        log.warning(f'No file found in: {root_wc}')

    return set(l_path)
# ---------------------------------
def main():
    '''
    Start here
    '''
    _initialize()
    l_sample = glob.glob(f'{Data.data_dir}/*')
    d_path   = {}

    log.info(40 * '-')
    log.info(f'{"Tree":<20}{"Latest":<10}{"Files":<10}')
    log.info(40 * '-')
    for sample in l_sample:
        version = _version_from_path(path=sample)
        if not version:
            continue

        name = os.path.basename(sample)

        s_path = _paths_from_sample(path=sample, version=version)
        d_path[f'{name}_{version}'] = s_path
        npath  = len(s_path)

        log.info(f'{name:<20}{version:<10}{npath:<10}')
    log.info(40 * '-')
# ---------------------------------
if __name__ == '__main__':
    main()
