'''
Module containing the selection function, which returns a dictionary of cuts
'''
# pylint: disable=too-many-positional-arguments, too-many-arguments, import-error

import os

from dataclasses         import dataclass
from importlib.resources import files

import yaml
import ap_utilities.decays.utilities as aput
from dmu.logging.log_store  import LogStore

from rx_selection import truth_matching     as tm
from rx_selection import version_management as vman

log=LogStore.add_logger('rx_selection:selection')
#-----------------------
@dataclass
class Data:
    '''
    Class used to store share attributes
    '''
    l_project  = ['RK', 'RKst']
    l_analysis = ['EE', 'MM'  ]
    l_q2bin    = ['low', 'central', 'jpsi', 'psi2S', 'high']
#-----------------------
def selection(analysis : str, project : str, q2bin: str, process : str) -> dict[str,str]:
    '''
    Picks up sample name, trigger, etc, returns dictionary with selection

    analysis : EE or MM
    project  : RK or RKst
    q2bin    : low, central, jpsi, psi2S or high
    process  : Nickname for MC sample, starts with "DATA" for data
    '''
    d_cut : dict[str,str] = {}

    event_type     = process if process.startswith('DATA') else aput.read_event_type(nickname=process)
    log.info(f'{process:<40}{"->":20}{event_type:<20}')

    if process.startswith('DATA'):
        log.debug('Adding cleaning requirement for data')
        d_cut['clean'] = 'dataq == 1'
    else:
        log.debug('Adding truth matching requirement for MC')
        d_cut['truth'] = tm.get_truth(event_type)

    d_tmp = _get_selection(analysis, project, q2bin)
    d_cut.update(d_tmp)

    return d_cut
#-----------------------
def load_selection_config() -> dict:
    '''
    Returns dictionary with the latest selection config
    '''
    sel_wc = files('rx_selection_data').joinpath('selection/*.yaml')
    sel_wc = str(sel_wc)
    sel_dir= os.path.dirname(sel_wc)

    yaml_path = vman.get_last_version(
            dir_path     = sel_dir,
            extension    = 'yaml',
            version_only = False ,
            main_only    = False)

    log.info(f'Loading selection from: {yaml_path}')

    with open(yaml_path, encoding='utf-8') as ifile:
        d_sel = yaml.safe_load(ifile)

    return d_sel
#-----------------------
def _get_selection(chan : str, proj: str, q2_bin : str) -> dict[str,str]:
    cfg = load_selection_config()

    if proj not in cfg:
        raise ValueError(f'Cannot find {proj} in config')

    if chan not in cfg[proj]:
        raise ValueError(f'Cannot find {chan} in config section for {proj}')

    d_cut = cfg[proj][chan]

    d_new = {}
    for cut_name, d_q2bin in d_cut.items():
        if not isinstance(d_q2bin, dict):
            d_new[cut_name] = d_q2bin
            continue

        if q2_bin not in d_q2bin:
            raise ValueError(f'Cannot find q2bin {q2_bin} in {cut_name} section')

        cut_val = d_q2bin[q2_bin]
        log.debug(f'Overriding {cut_name} for {q2_bin}')

        d_new[cut_name] = cut_val

    return d_new
#-----------------------
