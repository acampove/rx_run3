'''
Module containing functions used to find latest, next version, etc of a path.
'''

import glob
import os
import re

from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_selection:version_management')

#---------------------------------------
def _get_numeric_version(version : str) -> int:
    '''
    Takes string with numbers at the end (padded or not)
    Returns integer version of numbers
    '''
    #Skip these directories
    if version in ['__pycache__']:
        return -1

    regex=r'[a-z]+(\d+)'
    mtch =re.match(regex, version)
    if not mtch:
        log.debug(f'Cannot extract numeric version from: {version}')
        return -1

    str_val = mtch.group(1)
    val     = int(str_val)

    return val
#---------------------------------------
def get_last_version(
        dir_path     : str,
        extension    : str,
        version_only : bool = True,
        main_only    : bool = False) -> str:
    '''
    Returns path or just version associated to latest version found in given path

    Parameters
    ---------------------
    dir_path (str) : Path to directory where versioned subdirectories exist
    version_only (bool): Returns only vxxxx if True, otherwise, full path to directory
    main_only (bool): Returns vX where X is a number. Otherwise it will return vx.y in case version has subversion
    extension (str) : Extension of text files for which the latest version will be found, e.g. yaml

    Return
    ---------------------
    String with name of latest version, e.g. v1
    '''
    file_wc = f'{dir_path}/*.{extension}'
    l_obj   = glob.glob(file_wc)

    if len(l_obj) == 0:
        raise ValueError(f'Nothing found in {file_wc}')

    d_file_org = { os.path.basename(obj).replace('.', '') : obj       for obj             in l_obj             if os.path.isfile(obj) }
    d_file_num = {_get_numeric_version(name)              : file_path for name, file_path in d_file_org.items() }

    c_file = sorted(d_file_num.items())

    try:
        _, path = c_file[-1]
    except Exception as exc:
        raise ValueError(f'Cannot find path in: {file_wc}') from exc

    name = os.path.basename(path)
    dirn = os.path.dirname(path)

    if main_only and '.' in name:
        ind = name.index('.')
        name= name[:ind]

    if version_only:
        name = name.replace(f'.{extension}', '')
        return name

    return f'{dirn}/{name}'
#---------------------------------------
