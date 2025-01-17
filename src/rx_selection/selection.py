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

    event_type     = process if process.startswith('DATA_') else aput.read_event_type(nickname=process)
    log.info(f'{process:<40}{"->":20}{event_type:<20}')

    if process.startswith('DATA_'):
        d_cut['clean'] = 'dataq == 1'
    else:
        d_cut['truth'] = tm.get_truth(event_type)

    d_tmp = _get_selection(analysis, project, q2bin)
    d_cut.update(d_tmp)

    return d_cut
#-----------------------
def load_selection_config() -> dict:
    '''
    Returns dictionary with configuration (cuts, definitions, etc) needed for selection
    '''
    sel_wc = files('rx_selection_data').joinpath('selection/*.yaml')
    sel_wc = str(sel_wc)
    sel_dir= os.path.dirname(sel_wc)

    yaml_path = vman.get_last_version(
            dir_path     = sel_dir,
            extension    = 'yaml',
            version_only = False ,
            main_only    = False)

    with open(yaml_path, encoding='utf-8') as ifile:
        d_sel = yaml.safe_load(ifile)

    return d_sel
#-----------------------
def _get_selection(analysis : str, project : str, q2bin : str) -> dict[str,str]:
    d_sel  = load_selection_config()
    d_cut  = d_sel[project][analysis]
    q2_cut = d_cut['q2'  ][q2bin]
    ms_cut = d_cut['mass'][q2bin]

    del d_cut['q2'  ]
    del d_cut['mass']

    d_cut['q2'  ] = q2_cut
    d_cut['mass'] = ms_cut

    return d_cut
#-----------------------
