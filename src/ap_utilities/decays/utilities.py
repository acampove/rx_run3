'''
Module containing utility functions
'''
from importlib.resources import files
from functools           import cache

import yaml

# ---------------------------------
@cache
def _get_evt_name() -> dict[str,str]:
    file_path = files('ap_utilities_data').joinpath('evt_name.yaml')
    file_path = str(file_path)
    with open(file_path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# ---------------------------------
def _format_nickname(nickname : str, style : str) -> str:
    if style == 'literal':
        return nickname

    if style != 'safe_1':
        raise ValueError(f'Invalid style: {style}')

    nickname = nickname.replace(                 '.',     'p')
    nickname = nickname.replace(                 '-',    'mn')
    nickname = nickname.replace(                 '+',    'pl')
    nickname = nickname.replace(                 '=',  '_eq_')
    nickname = nickname.replace(                 ',',     '_')
    nickname = nickname.replace(        'DecProdCut',   'DPC')
    nickname = nickname.replace('EvtGenDecayWithCut', 'EGDWC')

    return nickname
# ---------------------------------
def read_decay_name(event_type : str, style : str = 'safe_1') -> str:
    '''
    Takes event type, and style strings, returns nickname of decay as defined in DecFiles package

    Styles:

    literal         : No change is made to nickname
    safe_1 (default): With following replacements:
        . -> p
        = -> _eq_
        - -> mn
        + -> pl
        , -> _
    '''
    d_evt_name = _get_evt_name()

    if event_type not in d_evt_name:
        raise ValueError(f'Event type {event_type} not found')

    value = d_evt_name[event_type]
    value = _format_nickname(value, style)

    return value
# ---------------------------------
