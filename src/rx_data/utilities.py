'''
Module with utility functions
'''
import os
import re

from dataclasses            import dataclass
from dmu.logging.log_store  import LogStore

log   = LogStore.add_logger('rx_data:utilities')


# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    # pylint: disable = invalid-name
    # Need to call var Max instead of max

    dt_rgx  = r'(data_\d{2}_.*c\d)_(Hlt2RD_.*(?:EE|MuMu|misid|cal|MVA|LL|DD))_?(\d{3}_\d{3}|[a-z0-9]{10})?\.root'
    mc_rgx  = r'mc_.*_\d{8}_(.*)_(\w+RD_.*)_(\d{3}_\d{3}|\w{10}).root'
# ---------------------------------
def info_from_path(path : str) -> tuple[str,str]:
    '''
    Will pick a path to a ROOT file
    Will return tuple with information associated to file
    This is needed to name output file and directories
    '''

    name = os.path.basename(path)
    if   name.startswith('dt_') or name.startswith('data_'):
        info = _info_from_data_path(path)
    elif name.startswith('mc_'):
        info = _info_from_mc_path(path)
    else:
        log.error(f'File name is not for data or MC: {name}')
        raise ValueError

    return info
# ---------------------------------
def _info_from_mc_path(path : str) -> tuple[str,str]:
    '''
    Will return information from path to file
    '''
    name = os.path.basename(path)
    mtch = re.match(Data.mc_rgx, name)
    if not mtch:
        raise ValueError(f'Cannot extract information from MC file:\n\n{name}\n\nUsing {Data.mc_rgx}')

    try:
        [sample, line, _] = mtch.groups()
    except ValueError as exc:
        raise ValueError(f'Expected three elements in: {mtch.groups()}') from exc

    return sample, line
# ---------------------------------
def _info_from_data_path(path : str) -> tuple[str,str]:
    '''
    Will get info from data path
    '''
    name = os.path.basename(path)
    mtch = re.match(Data.dt_rgx, name)
    if not mtch:
        raise ValueError(f'Cannot find kind in:\n\n{name}\n\nusing\n\n{Data.dt_rgx}')

    try:
        [sample, line, _] = mtch.groups()
    except ValueError as exc:
        raise ValueError(f'Expected three elements in: {mtch.groups()}') from exc

    sample = sample.replace('_turbo_', '_')
    sample = sample.replace('_full_' , '_')

    return sample, line
