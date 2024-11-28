'''
Script meant to read information from files in

https://gitlab.cern.ch/lhcb-datapkg/Gen/DecFiles

and store it in current project as data
'''
import os
import re
import glob
from dataclasses           import dataclass
from importlib.resources   import files

import tqdm
import yaml
from ap_utilities.logging.log_store import LogStore

log=LogStore.add_logger('ap_utilities_scripts:update_decinfo')
# ------------------------------
@dataclass
class Data:
    '''
    Class used to store shared data
    '''
    dec_path : str
    regex    : str = r'#[\s]*[a-zA-Z]+:[\s]*(.*)'
# ------------------------------
def _setup() -> None:
    if 'DECPATH' not in os.environ:
        raise ValueError('DECPATH, path to root of DecFiles, not found')

    Data.dec_path = os.environ['DECPATH']
# ------------------------------
def _line_from_list(file_path : str, contains : str, l_line : list[str]) -> str:
    l_value = [ line for line in l_line if contains in line ]

    if len(l_value) == 0:
        log.warning(f'Could not extract {contains} line in: {file_path}')
        return 'not_found'

    return l_value[0]
# ------------------------------
def _val_from_line(file_path : str, line : str) -> str:
    if line == 'not_found':
        return line

    mtch = re.match(Data.regex, line)
    if not mtch:
        log.warning(f'Cannot extract value from \"{line}\" in file {file_path}')
        return 'not_found'

    value = mtch.group(1)
    value = value.replace(' ', '')

    return value
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
    nfiles      = len(l_dec_file)
    if nfiles == 0:
        raise ValueError(f'No dec file foudn in {dec_file_wc}')

    log.info(f'Found {nfiles} decay files')

    l_evt_name = [ _get_evt_name(file_path) for file_path in tqdm.tqdm(l_dec_file, ascii=' -') ]
    d_evt_name = _dict_from_tup_list(l_evt_name)

    return d_evt_name
# ------------------------------
def _dict_from_tup_list(l_evt_name : list[tuple[str,str]]) -> dict[str,str]:
    d_res = {}
    for key, val in l_evt_name:
        if key in d_res:
            old_val = d_res[key]
            log.warning(f'Key {key} with value {old_val} already found, overriding with {val}')

        d_res[key] = val

    return d_res
# ------------------------------
def _dump_info(d_evt_name : dict[str,str]) -> None:
    yaml_path = files('dmu_data').joinpath('physics/evt_name.yaml')
    yaml_path = str(yaml_path)

    log.info(f'Saving to: {yaml_path}')
    with open(yaml_path, 'w', encoding='utf-8') as ofile:
        yaml.dump(d_evt_name, ofile)
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
