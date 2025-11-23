'''
Module with Dependencies class
'''
from pathlib   import Path
from typing    import Any
from functools import cached_property

from dmu.logging.log_store import LogStore
from dmu.generic           import hashing

log=LogStore.add_logger('dependencies')
# ----------------------
class Dependencies:
    '''
    Class meant to be used as a mixin for law Task classes
    It adds functionality to track changes in inputs
    '''
    dependencies : list
    # ----------------------------------
    def _hash_from_obj(self, obj : Any) -> str:
        '''
        Parameters
        -------------
        obj: Arbitrary object meant to be hashed

        Returns
        -------------
        hash string associated
        '''
        if isinstance(obj, Path):
            return hashing.hash_file(path=obj)

        raise TypeError('Invalid type')
    # ----------------------------------
    @cached_property
    def hash_path(self) -> Path:
        if not hasattr(self, 'dependencies'):
            raise AttributeError('Cannot find "dependencies" attribute')

        if not isinstance(self.dependencies, tuple):
            raise TypeError(f'Dependencies attribute is not a tuple: {self.dependencies}')

        l_hash = []
        for dependency in self.dependencies:
            hsh = self._hash_from_obj(obj=dependency)
            l_hash.append(hsh)

        hsh = hashing.hash_object(obj=l_hash)

        return Path(f'/tmp/{hsh}')
    # ----------------------
    def _touch_hash(self) -> None:
        '''
        Write a file with the hash in the name
        '''
        self.hash_path.touch()
# ----------------------
