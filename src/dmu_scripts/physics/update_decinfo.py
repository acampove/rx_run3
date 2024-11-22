'''
Script meant to read information from files in

https://gitlab.cern.ch/lhcb-datapkg/Gen/DecFiles

and store it in current project as data
'''
import os
import re
import glob
from dataclasses           import dataclass

import tqdm
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu_scripts:physics:update_decinfo')
# ------------------------------
@dataclass
class Data:
    '''
    Class used to store shared data
    '''
    dec_path : str
    regex    : str = r'# [a-zA-Z]+: (.*)'
# ------------------------------
def _setup() -> None:
    if 'DECPATH' not in os.environ:
        raise ValueError('DECPATH, path to root of DecFiles, not found')

    Data.dec_path = os.environ['DECPATH']
# ------------------------------
def _line_from_list(file_path : str, contains : str, l_line : list[str]) -> str:
    try:
        [value] = [line for line in l_line if contains in line]
    except ValueError:
        log.warning(f'Could not extract {contains} line in: {file_path}')
        return 'not_found'

    return value
# ------------------------------
def _val_from_line(file_path : str, line : str) -> str:
    mtch = re.match(Data.regex, line)
    if not mtch:
        log.warning(f'Cannot extract value from {line} in file {file_path}')
        return 'not_found'

    return mtch.group(1)
# ------------------------------
def _get_evt_name(file_path : str) -> tuple[str,str]:
    with open(file_path, encoding='utf-8') as ifile:
        l_line = ifile.read().splitlines()

    evt_line = _line_from_list(file_path, 'EventType', l_line)
    nam_line = _line_from_list(file_path, 'NickName' , l_line)

    evt_type = _val_from_line(file_path, evt_line)
    nickname = _val_from_line(file_path, nam_line)

    return evt_type, nickname
# ------------------------------
def _read_info() -> dict[str,str]:
    dec_file_wc = f'{Data.dec_path}/dkfiles/*.dec'
    l_dec_file  = glob.glob(dec_file_wc)
    l_dec_file  = l_dec_file[:20]
    nfiles      = len(l_dec_file)
    if nfiles == 0:
        raise ValueError(f'No dec file foudn in {dec_file_wc}')

    log.info(f'Found {nfiles} decay files')

    l_evt_name = [ _get_evt_name(file_path) for file_path in tqdm.tqdm(l_dec_file, ascii=' -') ]
    d_evt_name = dict(l_evt_name)

    info_size = len(d_evt_name)

    if info_size != nfiles:
        raise ValueError(f'Number of files and size of dictionary differ: {nfiles}/{info_size}')

    return d_evt_name
# ------------------------------
def _dump_info(d_evt_name : dict[str,str]) -> None:
    ...
# ------------------------------
def main():
    '''
    Script starts here
    '''
    _setup()
    d_evt_name = _read_info()
    _dump_info(d_evt_name)
# ------------------------------
if __name__ == '__main__':
    main()
