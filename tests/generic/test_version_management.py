'''
Module containing tests for version management functions
'''
import os
import shutil

from dmu.generic.version_management import get_last_version
from dmu.generic.version_management import get_next_version
from dmu.generic.version_management import get_latest_file
from dmu.logging.log_store          import LogStore

log=LogStore.add_logger('dmu:test_version_management')
#-----------------------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir = '/tmp/dmu/tests/version_manager/'
    nver    = 19
#-----------------------------------------------------------
def _create_files(nfiles : int) -> str:
    '''
    Helper function that makes files
    '''
    test_dir = f'{Data.out_dir}/latest_file'
    os.makedirs(test_dir, exist_ok=True)

    for ind in range(nfiles):
        file_path = f'{test_dir}/name_v{ind}.txt'
        with open(file_path, 'w', encoding='utf-8') as ofile:
            ofile.close()

    return test_dir
#-----------------------------------------------------------
def _get_dir(name) -> tuple[str,str]:
    path_dir = f'{Data.out_dir}/{name}'
    shutil.rmtree(path_dir, ignore_errors=True)
    for ver in range(Data.nver + 1):
        os.makedirs(f'{path_dir}/v{ver}', exist_ok=True)

    if name == 'non_numeric':
        os.makedirs(f'{path_dir}/vx', exist_ok=True)
        os.makedirs(f'{path_dir}/vy', exist_ok=True)
        os.makedirs(f'{path_dir}/vz', exist_ok=True)

    return path_dir, f'v{ver}'
#-----------------------------------------------------------
def test_nonnumeric():
    '''
    Tests getting last version
    '''
    dir_path, iversion = _get_dir('nonnumeric')
    oversion=get_last_version(dir_path=dir_path, version_only=True)

    assert iversion == oversion
#-----------------------------------------------------------
def test_simple():
    '''
    Tests getting last version
    '''
    dir_path, iversion = _get_dir('simple')
    oversion=get_last_version(dir_path=dir_path, version_only=True)

    assert iversion == oversion
#-----------------------------------------------------------
def test_files():
    '''
    Testing getting the latest file, from its version
    '''
    file_dir  = _create_files(5)
    last_file = get_latest_file(dir_path = file_dir, wc='name_*.txt')

    assert last_file == f'{Data.out_dir}/latest_file/name_v4.txt'
#-----------------------------------------------------------
def test_next():
    '''
    Tests getting next version
    '''
    assert get_next_version('v1')           == 'v2'
    assert get_next_version('v1.1')         == 'v2.1'
    assert get_next_version('v10.1')        == 'v11.1'

    assert get_next_version('/a/b/c/v1')    == '/a/b/c/v2'
    assert get_next_version('/a/b/c/v1.1')  == '/a/b/c/v2.1'
    assert get_next_version('/a/b/c/v10.1') == '/a/b/c/v11.1'
#-----------------------------------------------------------
