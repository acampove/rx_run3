'''
Module containing utility functions
'''
from importlib.resources import files

import yaml

# ---------------------------------
def _load_data(file_name : str) -> dict:
    file_path = files('ap_utilities_data').joinpath(file_name)
    file_path = str(file_path)
    with open(file_path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# ---------------------------------
def format_nickname(nickname : str, style : str) -> str:
    '''
    Function taking decays nickname and returning formatted version

    nickaname: Name to be formatted
    style    : How to format name, supported: literal, safe_1
    '''
    if style == 'literal':
        return nickname

    if style != 'safe_1':
        raise ValueError(f'Invalid style: {style}')

    nickname = nickname.replace(                  '.',     'p')
    nickname = nickname.replace(                  '-',    'mn')
    nickname = nickname.replace(                  '+',    'pl')
    nickname = nickname.replace(                  '=',  '_eq_')
    nickname = nickname.replace(                  ',',     '_')
    nickname = nickname.replace(         'DecProdCut',   'DPC')
    nickname = nickname.replace( 'EvtGenDecayWithCut', 'EGDWC')
    nickname = nickname.replace('VisibleInAcceptance',   'VIA')
    nickname = nickname.replace(        'HighVisMass',   'HVM')
    nickname = nickname.replace(       'OppositeSign',    'OS')
    nickname = nickname.replace(           'TightCut',    'TC')
    nickname = nickname.replace('DiMuon_OppositeSign','DiM_OS')
    nickname = nickname.replace('GeV'                ,     'G')

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
    d_evt_name = _load_data('evt_name.yaml')

    if event_type not in d_evt_name:
        raise ValueError(f'Event type {event_type} not found')

    value = d_evt_name[event_type]
    value = format_nickname(value, style)

    return value
# ---------------------------------
def read_event_type(nickname : str) -> str:
    '''
    Takes nickname after reformatting, i.e. replacement of commans, equals, etc.
    Returns corresponding event type 
    '''
    d_name_evt = _load_data('name_evt.yaml')

    if nickname not in d_name_evt:
        raise ValueError(f'Event type {nickname} not found')

    value = d_name_evt[nickname]

    return value
# ---------------------------------
def new_from_old_nick(nickname : str, style : str = 'safe_1') -> str:
    '''
    Function that takes a decay nick name using Run1/2 naming
    and returns nicknames using Run3 naming
    '''
    d_old_evt = _load_data('old_name_evt.yaml')
    if nickname not in d_old_evt:
        raise ValueError(f'Old nickname {nickname} not found in: old_name_evt.yaml')

    evt_type   = d_old_evt[nickname]

    d_evt_name = _load_data('evt_name.yaml')
    if evt_type not in d_evt_name:
        raise ValueError(f'Event type {evt_type} not found in: evt_name.yaml')

    new_nick   = d_evt_name[evt_type]
    new_nick   = format_nickname(new_nick, style)

    return new_nick
# ---------------------------------
def old_from_new_nick(nickname : str) -> str:
    '''
    Function that takes a decay nick name using Run3 naming with safe_1 style
    and returns nicknames using Run1/2 naming
    '''
    d_name_evt = _load_data('name_evt.yaml')
    if nickname not in d_name_evt:
        raise ValueError(f'Nickname {nickname} not found in: name_evt.yaml')

    evt_type   = d_name_evt[nickname]

    d_evt_old  = _load_data('evt_old_name.yaml')
    if evt_type not in d_evt_old:
        raise ValueError(f'Event type {evt_type} not found in: evt_old_name.yaml')

    old_nick   = d_evt_old[evt_type]

    return old_nick
