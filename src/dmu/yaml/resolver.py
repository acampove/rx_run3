'''
This module contains the YamlResolver class
'''

from typing import Mapping


class Resolver:
    '''
    This class will:

    - Take a dictionary mapping simple types (int or str) to strings
    - The strings in the values will contain fields like {var} where `var`
      is a key
    - The instance will return the value for a given key, with all
      the variales resolved
    '''
    # ----------------------
    def __init__(self, cfg : Mapping[str|int,str]) -> None:
        '''
        Parameters
        -------------
        cfg: Dictionary like object with potentially unresolved, but resolvable fields
        '''
        self._cfg = { str(key) : value for key, value in cfg.items()}
    # ----------------------
    def __call__(self, key : str|int) -> str:
        '''
        Parameters
        -------------
        key: Key of dictionary for which value is needed

        Returns
        -------------
        String corresponding to value of mapping
        '''
        key = str(key)

        if key not in self._cfg:
            raise KeyError(f'Cannot find {key}')

        expr = self._cfg[key]
        seen = set()
        prev = None
        while prev != expr:
            if expr in seen:
                raise ValueError(f'Circular reference detected while resolving: {expr}')

            seen.add(expr)

            prev = expr
            try:
                expr = expr.format(**self._cfg)
            except KeyError:
                break

        return expr
