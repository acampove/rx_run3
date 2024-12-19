'''
Module used to put RX core utilities, written in c++ in namespace
'''
import os
import re

from dataclasses import dataclass

import ROOT

# --------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''
    rgx_ldpath = r'.*-L(\/cvmfs.*\/lib).*'
    l_library  : list[str] = [
            'libfitter.so',
            'libkernel.so',
            'libroofit.so',
            'libtoys.so',
            'libtuples.so',
            'libvelo.so']
# --------------------------------
def _get_names() -> list[str]:
    ld_arg  = os.environ['LDFLAGS']
    mtch    = re.match(Data.rgx_ldpath, ld_arg)
    if not mtch:
        raise ValueError(f'Cannot extract libraries path from: {ld_arg}')

    l_lib_path = []
    ld_path    = mtch.group(1)
    for library in Data.l_library:
        lib_path = f'{ld_path}/{library}'
        if not os.path.isfile(lib_path):
            raise FileNotFoundError(f'Cannot find: {lib_path}')

        l_lib_path.append(lib_path)

    return l_lib_path
# --------------------------------
def _load_libraries(l_lib_path : list[str]) -> None:
    for lib_path in l_lib_path:
        print(f'Loading: {lib_path}')
        ROOT.gSystem.Load(lib_path)
# --------------------------------
def main():
    '''
    Script starts here
    '''
    l_name = _get_names()
    _load_libraries(l_name)
# --------------------------------
main()
