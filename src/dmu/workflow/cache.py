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
    def _get_dir(self, kind : str) -> str:
        '''
        Parameters
        --------------
        kind : Kind of directory, cash, hash
        '''
        if   kind == 'cache':
            dir_path  = f'{self._out_path}/.cache'
        elif kind == 'hash':
            cache_dir = self._get_dir(kind='cache')
            hsh       = hashing.hash_object(self._dat_hash)
            dir_path  = f'{cache_dir}/{hsh}'
        else:
            raise ValueError(f'Invalid directory kind: {kind}')

        os.makedirs(dir_path, exist_ok=True)

        return dir_path
    # ---------------------------
    def _cache(self) -> None:
        '''
        Meant to be called after all the calculations finish
        It will copy all the outputs of the processing
        to a hashed directory
        '''
        log.info('Caching outputs')

        self._hash_dir  = self._get_dir(kind= 'hash')
        for source in Path(self._out_path).glob('*'):
            if str(source) == self._cache_dir:
                continue

            log.debug(f'{str(source):<50}{"-->"}{self._hash_dir}')

            if source.is_dir():
                shutil.copytree(source, self._hash_dir)
            else:
                shutil.copy2(source, self._hash_dir)
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
