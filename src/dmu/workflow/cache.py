'''
This module contains
'''
import os
import shutil
from pathlib import Path

from dmu.generic           import hashing
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:workflow:cache')
# ---------------------------
class Cache:
    '''
    Class meant to wrap other classes in order to

    - Keep track of the inputs through hashes
    - Load cached data, if found, and prevent calculations

    The following directories will be important:

    out_dir  : Directory where the outputs will go, specified by the user
    cache_dir: Subdirectory of out_dir, ${out_dir}/.cache
    hash_dir : Subdirectory of out_dir, ${out_dir}/.cache/{hash}
               Where {hash} is a 10 alphanumeric representing the has of the inputs
    '''
    # ---------------------------
    def __init__(self, out_path : str, **kwargs):
        '''
        Parameters
        ---------------
        out_path: Path to directory where outputs will go
        kwargs  : Key word arguments symbolizing identity of inputs, used for hashing
        '''
        self._out_path  = out_path
        self._dat_hash  = kwargs

        self._cache_dir = self._get_dir(kind='cache')
        self._hash_dir  : str
    # ---------------------------
    def _get_hash(self) -> str:
        '''
        Create hash with all the collected user inputs
        '''
        log.debug('Will use following keys and values for hashing')
        for key, val in self._dat_hash.items():
            log.debug(f'{key:<30}{val}')

        hsh = hashing.hash_object(self._dat_hash)
        hsh = f'.hash_{hsh}.yaml'

        return hsh
    # ---------------------------
    def _set_output(self, dir_path : str) -> None:
        '''
        Parameters
        --------------
        dir_path: Path to directory where outputs will be cached
        '''
        os.makedirs(dir_path, exist_ok=True)
        self._dir_path = dir_path

        log.debug(f'Will send outputs to: {self._dir_path}')
    # ---------------------------
    def _mark_as_cached(self) -> None:
        '''
        Checks if hash file exists

        Yes: raises exception, if file exists, you would not be calling this method
        No : Makes the file and removes old hash files, output was updated
        '''
        hsh       = self._get_hash()
        hash_file = f'{self._dir_path}/{hsh}'
        if os.path.isfile(hash_file):
            raise ValueError(f'Hash file found: {hash_file}')

        for fpath in Path(self._dir_path).glob('.hash*.yaml'):
            log.debug(f'Removing old hash file: {fpath}')
            fpath.unlink()

        log.debug(f'Writtng hash file: {hash_file}')
        gut.dump_json(self._dat_hash, hash_file)
    # ---------------------------
    def _is_cached(self) -> bool:
        '''
        Checks if hash file (empty file whose name is the hash) exists in directory with outputs

        Returns
        ---------------
        True if the object, cached was found, false otherwise.
        '''
        hsh       = self._get_hash()
        hash_file = f'{self._dir_path}/{hsh}'
        found = os.path.isfile(hash_file)

        log.debug(f'Data was found: {found}')

        return found
# ---------------------------
