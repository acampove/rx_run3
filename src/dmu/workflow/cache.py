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
    def _delete_from_output(self) -> None:
        '''
        Delete all objects from _out_path directory, except for `.cache`
        '''
        for path in Path(self._out_path).iterdir():
            if str(path) == self._cache_dir:
                continue

            log.debug(f'Deleting {path}, cache dir {self._cache_dir}')
            log.debug(f'Cache dir: {self._cache_dir}')
            log.debug(f'This dir: {path}')

            if path.is_dir():
                shutil.rmtree(path)
            else:
                path.unlink()
    # ---------------------------
    def _copy_from_hashdir(self) -> None:
        '''
        Copies all the objects from _hash_dir to _out_path
        '''
        for source in Path(self._hash_dir).iterdir():
            target = f'{self._out_path}/{source.name}'

            log.debug(f'{source:<50}{"-->"}{target}')

            if source.is_dir():
                shutil.copytree(source, target)
            else:
                shutil.copy2(source, target)
    # ---------------------------
    def _copy_from_cache(self) -> bool:
        '''
        Checks if hash directory exists:

        No : Returns False
        Yes:
            - Removes contents of `out_path`, except for .cache
            - Copies the contents of `hash_dir` to `out_dir`

        Returns
        ---------------
        True if the object, cached was found, false otherwise.
        '''
        if not hasattr(self, '_hash_dir'):
            return False

        log.debug('Data found in hash directory: {self._hash_dir}')

        self._delete_from_output()
        self._copy_from_hashdir()

        return True
# ---------------------------
