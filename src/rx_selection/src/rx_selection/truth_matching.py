'''
Module containing functions used for truth matching
'''

from ap_utilities.decays   import utilities as aput
from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut
from dmu.yaml.resolver     import Resolver

log=LogStore.add_logger('rx_selection:truth_matching')
# ----------------------
def get_event_type(arg : int|str) -> str:
    '''
    Parameters
    -------------
    arg: Either event type or nickname for decay

    Returns
    -------------
    Event type
    '''
    if isinstance(arg, int):
        return str(arg)

    if arg.isdigit():
        return arg

    try:
        event_type = aput.read_event_type(nickname=arg)
    except ValueError as exc:
        raise ValueError(f'Argument {arg} is neither data nor can be interpreted into an event type') from exc

    return event_type
# ----------------------------------------------------------
def get_truth(arg : int|str, kind : str) -> str:
    r'''
    Parameters:
    --------------------------
    arg : Event type or decay nickname
    kind: Either bukll or bdkstll

    Returns:
    --------------------------
    For data it will return '(1)'. For MC, truth matching string. 
    The truth matching string will assume that the candidates were reconstructed as:

    kind = bdkstll -> $B^0\to K^* e^+e^-$ candidates
    kind = bukll   -> $B^+\to K^+ e^+e^-$ candidates
    '''
    if isinstance(arg, str) and arg.startswith('DATA_'):
        return '(1)'

    event_type = get_event_type(arg=arg)
    cfg        = gut.load_conf(package='rx_selection_data', fpath=f'truth_matching/{kind}.yaml')
    yrs        = Resolver(cfg=cfg)

    if event_type not in yrs:
        raise KeyError(f'Could not find event type {event_type} in config')

    requirement = yrs[event_type]

    log.debug(f'Using truth matching: {requirement}')

    return requirement
# ----------------------------------------------------------

