'''
This module contains
'''
import os
import sys
import shutil
from pathlib import Path

from git                   import Repo
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
    _cache_root : str|None = None
    # ---------------------------
    def __init__(self, out_path : str, **kwargs):
        '''
        Parameters
        ---------------
        out_path: Path to directory where outputs will go
        kwargs  : Key word arguments symbolizing identity of inputs, used for hashing
        '''
        if Cache._cache_root is None:
            raise ValueError('Caching directory not set')

        if 'code' in kwargs:
            raise ValueError('Cannot append hashing data with key "code", already used')

        kwargs['code']  = self._get_code_hash()

        self._out_path  = f'{Cache._cache_root}/{out_path}'
        self._dat_hash  = kwargs

        self._cache_dir = self._get_dir(kind='cache')
        self._hash_dir  : str
    # ---------------------------
    @classmethod
    def set_cache_root(cls, root : str) -> None:
        '''
        Sets the path to the directory WRT which the _out_path_
        will be placed
        '''
        if cls._cache_root is not None:
            raise ValueError(f'Trying to set {root}, but already found {cls._cache_root}')

        os.makedirs(root, exist_ok=True)

        cls._cache_root = root
    # ---------------------------
    def _get_code_hash(self) -> str:
        '''
        If `MyTool` inherits from `Cache`. `mytool.py` git commit hash
        should be returned
        '''
        repo  = Repo('.')
        cls   = self.__class__
        mod   = sys.modules.get(cls.__module__)
        if mod is None:
            raise ValueError(f'Module not found: {cls.__module__}')

        fname = str(mod.__file__)
        fpath = os.path.abspath(fname)

        genr=repo.iter_commits(paths=fpath, max_count=1)

        [hsh] = list(genr)
        val   = hsh.hexsha

        log.debug(f'Using hash for: {fpath} = {val}')

        return val
    # ---------------------------
    def _get_dir(
            self,
            kind : str,
            make : bool = True) -> str:
        '''
        Parameters
        --------------
        kind : Kind of directory, cash, hash
        make : If True (default) will try to make directory
        '''
        if   kind == 'cache':
            dir_path  = f'{self._out_path}/.cache'
        elif kind == 'hash':
            cache_dir = self._get_dir(kind='cache')
            hsh       = hashing.hash_object(self._dat_hash)
            dir_path  = f'{cache_dir}/{hsh}'
        else:
            raise ValueError(f'Invalid directory kind: {kind}')

        if make:
            os.makedirs(dir_path, exist_ok=True)

        return dir_path
    # ---------------------------
    def _cache(self) -> None:
        '''
        Meant to be called after all the calculations finish
        It will copy all the outputs of the processing
        to a hashed directory
        '''
        self._hash_dir  = self._get_dir(kind= 'hash')
        log.info(f'Caching outputs to: {self._hash_dir}')

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
                log.debug(f'Skipping cache dir: {self._cache_dir}')
                continue

            # These will always be symbolic links
            if not path.is_symlink():
                log.warning(f'Found a non-symlink not deleting: {path}')
                continue

            log.debug(f'Deleting {path}')
            path.unlink()
    # ---------------------------
    def _copy_from_hashdir(self) -> None:
        '''
        Copies all the objects from _hash_dir to _out_path
        '''
        for source in Path(self._hash_dir).iterdir():
            target = f'{self._out_path}/{source.name}'
            log.debug(f'{str(source):<50}{"-->"}{target}')

            os.symlink(source, target)
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
        hash_dir = self._get_dir(kind='hash', make=False)
        if not os.path.isdir(hash_dir):
            log.debug(f'Hash directory {hash_dir} not found, not caching')
            self._delete_from_output()
            return False

        self._hash_dir = hash_dir
        log.debug(f'Data found in hash directory: {self._hash_dir}')

        self._delete_from_output()
        self._copy_from_hashdir()

        return True
# ---------------------------
