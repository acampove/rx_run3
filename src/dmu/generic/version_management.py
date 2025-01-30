'''
Module containing functions used to find latest, next version, etc of a path.
'''

import glob
import os
import re

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:version_management')
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
def get_last_version(dir_path : str, version_only : bool = True, main_only : bool = False):
    '''Returns path or just version associated to latest version found in given path

    Parameters
    ---------------------
    dir_path (str) : Path to directory where versioned subdirectories exist
    version_only (bool): Returns only vxxxx if True, otherwise, full path to directory
    main_only (bool): Returns vX where X is a number. Otherwise it will return vx.y in case version has subversion
    '''
    l_obj = glob.glob(f'{dir_path}/*')

    if len(l_obj) == 0:
        log.error(f'Nothing found in {dir_path}')
        raise ValueError

    d_dir_org = { os.path.basename(obj).replace('.', '') : obj for obj in l_obj if os.path.isdir(obj) }
    d_dir_num = { _get_numeric_version(name) : dir_path for name, dir_path in d_dir_org.items() }

    c_dir = sorted(d_dir_num.items())

    try:
        _, path = c_dir[-1]
    except:
        log.error(f'Cannot find path in: {dir_path}')
        raise

    name = os.path.basename(path)
    dirn = os.path.dirname(path)

    if main_only and '.' in name:
        ind = name.index('.')
        name= name[:ind]

    if version_only:
        return name

    return f'{dirn}/{name}'
#---------------------------------------
def get_latest_file(dir_path : str, wc : str) -> str:
    '''Will find latest file in a given directory

    Parameters
    --------------------
    dir_path (str): Directory where files are found
    wc (str): Wildcard associated to files, e.g. file_*.txt

    Returns
    --------------------
    Path to latest file, according to version
    '''
    l_path = glob.glob(f'{dir_path}/{wc}')
    if len(l_path) == 0:
        log.error(f'Cannot find files in: {dir_path}/{wc}')
        raise ValueError

    l_path.sort()

    return l_path[-1]
#---------------------------------------
def get_next_version(version : str) -> str:
    '''Pick up string symbolizing version and return next version
    Parameters
    -------------------------
    version (str) : Of the form vx.y or vx where x and y are integers. It can also be a full path

    Returns
    -------------------------
    String equal to the argument, but with the main version augmented by 1, e.g. vx+1.y

    Examples:
    -------------------------

    get_next_version('v1.1') = 'v2.1'
    get_next_version('v1'  ) = 'v2'
    '''
    if '/' in version:
        path    = version
        dirname = os.path.dirname(path)
        version = os.path.basename(path)
    else:
        dirname = None

    rgx = r'v(\d+)(\.\d+)?'

    mtch = re.match(rgx, version)
    if not mtch:
        log.error(f'Cannot match {version} with {rgx}')
        raise ValueError

    ver_org = mtch.group(1)
    ver_nxt = int(ver_org) + 1
    ver_nxt = str(ver_nxt)

    version = version.replace(f'v{ver_org}', f'v{ver_nxt}')

    if dirname is not None:
        version = f'{dirname}/{version}'

    return version
#---------------------------------------
