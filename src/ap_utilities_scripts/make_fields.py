'''
Script used to build decay fields from YAML file storing event type -> decay correspondence
'''
import re
import argparse
from typing                         import Union
from importlib.resources            import files

import yaml
from ap_utilities.logging.log_store import LogStore

log = LogStore.add_logger('ap_utilities:make_fields')
# ---------------------------
class Data:
    '''
    Class storing shared data
    '''
    l_skip_type= [
            '12952000',
            '11453001',
            '13454001',
            '15454101',
            '12442001',
            '11442001',
            '13442001',
            '15444001',
            ]

    d_repl_sym = {
            'cc'        :      'CC',
            '->'        :     '==>',
            }

    d_repl_par = {
            'psi(2S)'  :   'psi_2S_',
            'psi(1S)'  :   'psi_1S_',
            'K*(892)'  :   'K*_892_',
            'phi(1020)': 'phi_1020_',
            }

    d_repl_spa = {
            '('        :     ' ( ',
            ')'        :     ' ) ',
            '['        :     ' [ ',
            ']'        :     ' ] ',
            }

    l_event_type : list[str]
    d_decay      : dict[str,str]
# ---------------------------
def _load_decays() -> None:
    dec_path = files('ap_utilities_data').joinpath('evt_dec.yaml')
    dec_path = str(dec_path)
    with open(dec_path, encoding='utf-8') as ifile:
        Data.d_decay = yaml.safe_load(ifile)
# ---------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-i', '--input' , type=str, help='Path to textfile with event types')
    args = parser.parse_args()

    input_path = args.input
    with open(input_path, encoding='utf-8') as ifile:
        Data.l_event_type = ifile.read().splitlines()
# ---------------------------
def _reformat_decay(decay : str) -> str:
    # Symbol renaming needed, e.g. -> ==>, cc -> CC
    for org, new in Data.d_repl_sym.items():
        decay = decay.replace(org, new)

    # Need to make special substrings into underscored ones
    # e.g. J/psi(1S) -> J/psi_1S_
    for org, new in Data.d_repl_par.items():
        decay = decay.replace(org, new)

    # Add spaces to parentheses and brackets
    for org, new in Data.d_repl_spa.items():
        decay = decay.replace(org, new)

    return decay
# ---------------------------
def _reformat_back_decay(decay : str) -> str:
    # Put back special characters original naming
    for org, new in Data.d_repl_par.items():
        decay = decay.replace(new, org)

    # Decay cannot have space here, other spaces are allowed
    decay = decay.replace('] CC', ']CC')

    decay = decay.replace('[ '  ,   '[')
    decay = decay.replace('[  ' ,   '[')
    decay = decay.replace('  ]' ,   ']')

    decay = decay.replace('(  ' ,   '(')
    decay = decay.replace('  )' ,   ')')

    return decay
# ---------------------------
def _replace_back(part : str) -> str:
    for org, new in Data.d_repl_par.items():
        if new in part:
            part = part.replace(new, org)

    return part
# ---------------------------
def _particles_from_decay(decay : str) -> list[str]:
    l_repl = list(Data.d_repl_sym.values())
    l_repl+= list(Data.d_repl_spa.values())
    l_repl = [ repl.replace(' ', '') for repl in l_repl ]

    l_part = decay.split(' ')
    l_part = [ part for part in l_part if part not in l_repl ]
    l_part = [ part for part in l_part if part != ''         ]
    l_part = [ _replace_back(part) for part in l_part ]

    return l_part
# ---------------------------
def _skip_decay(event_type : str, decay : str) -> bool:
    if event_type in Data.l_skip_type:
        return True

    if '{,gamma}' in decay:
        return True

    if 'nos' in decay:
        return True

    return False
# ---------------------------
def _get_hatted_decay( particle : str, decay : str) -> str:
    return decay
# ---------------------------
def _get_decays(event_type : str) -> Union[None,dict[str,str]]:
    decay = Data.d_decay[event_type]
    if _skip_decay(event_type, decay):
        return None

    decay = _reformat_decay(decay)
    l_par = _particles_from_decay(decay)
    l_par = _rename_repeated(l_par)
    decay = _reformat_back_decay(decay)

    d_dec = {}
    for i_par, par in enumerate(l_par):
        d_dec[par] = _get_hatted_decay(par, i_par, decay)

    return d_dec
# ---------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _load_decays()
    for event_type in Data.l_event_type:
        d_tmp = _get_decays(event_type)
        if d_tmp is None:
            continue

        for key, val in d_tmp.items():
            print(f'{key:<20}{val:<100}')

        print('')
# ---------------------------
if __name__ == '__main__':
    main()
