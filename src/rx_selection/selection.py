'''
Module containing the selection function, which returns a dictionary of cuts
'''
# pylint: disable=too-many-positional-arguments, too-many-arguments, import-error

import os

from dataclasses         import dataclass
from importlib.resources import files

import yaml
import ap_utilities.decays.utilities as aput
from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore

from rx_data      import utilities          as dut
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

    d_custom_selection : dict[str,str]
#-----------------------
class MultipleSelectionOverriding(Exception):
    '''
    Will be risen when global selection is overriden more than once per run
    '''
    def __init__(self, message):
        super().__init__(message)
#-----------------------
def _print_selection(d_cut : dict[str,str]) -> None:
    for name, expr in d_cut.items():
        log.debug(f'{name:<20}{expr}')
#-----------------------
def _get_analysis(trigger : str) -> str:
    if dut.is_ee(trigger=trigger):
        return 'EE'

    return 'MM'
#-----------------------
def _project_from_trigger(trigger : str, project : str) -> str:
    if trigger is None:
        log.warning('It is advised to specify the trigger rather than the analysis')
        return project

    if trigger.startswith('Hlt2RD_BuToKpMuMu_MVA') or trigger.startswith('Hlt2RD_BuToKpEE_MVA'):
        return 'RK'

    raise NotImplementedError(f'Cannot deduce project for trigger: {trigger}')
#-----------------------
def set_custom_selection(d_cut : dict[str,str]) -> None:
    '''
    This function is meant to override the analysis selection, such that

    - Every tool that uses the selection picks up the same selection
    - Cuts are added (e.g. brem category) or modified (e.g. MVA WP)

    This function is meant to be called once, at the beginning of the process, e.g. main function.
    '''
    if hasattr(Data, 'd_custom_selection'):
        raise MultipleSelectionOverriding('Custom selection can only be set once')

    log.warning('Setting custom selection')
    for cut_name, cut_expr in d_cut.items():
        log.info(f'{cut_name:<20}{cut_expr}')

    Data.d_custom_selection = d_cut
#-----------------------
def reset_custom_selection() -> None:
    '''
    This function can be called to remove the custom selection.

    Why should I need this?

    Mostly because you want to try different selections in parametrized tests
    If used elsewhere it might lead to an analysis running different selections in different
    parts of the code
    '''

    if not hasattr(Data, 'd_custom_selection'):
        log.warning('No custom selection found')
        return

    log.warning('Resetting custom selection')

    del Data.d_custom_selection
#-----------------------
# TODO: Eventually we should only require trigger, project and analysis can be deduced from that.
def selection(
        q2bin    : str,
        process  : str,
        project  : str = None,
        smeared  : bool= True,
        trigger  : str = None) -> dict[str,str]:
    '''
    Picks up sample name, trigger, etc, returns dictionary with selection

    project  : Optional, RK or RKst, if not passed, will be decided from trigger argument
    q2bin    : low, central, jpsi, psi2S or high
    process  : Nickname for MC sample, starts with "DATA" for data
    trigger  : E.g. Hlt2RD...
    smeared  : If true (default), the selection will use cuts on smeared masses. Only makes sense for electron MC samples
    '''

    project  = _project_from_trigger(trigger=trigger, project=project)
    analysis = _get_analysis(trigger)

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

    if hasattr(Data, 'd_custom_selection'):
        d_cut.update(Data.d_custom_selection)

    if dut.is_mc(sample=process) and dut.is_ee(trigger=trigger) and smeared:
        d_cut = _use_smeared_masses(cuts=d_cut, q2bin=q2bin)
    else:
        log.warning('Using cuts on un-smeared masses')

    _print_selection(d_cut)

    return d_cut
#-----------------------
def _use_smeared_masses(cuts : dict[str,str], q2bin : str) -> dict[str,str]:
    log.info('Overriding selection for electron MC to use smeared q2 and mass')

    cut_org    = cuts['q2']
    cut_new    = cut_org.replace('q2', 'q2_smr')
    cuts['q2'] = cut_new

    log.debug('Overriding:')
    log.debug(cut_org)
    log.debug('--->')
    log.debug(cut_new)

    if dut.is_reso(q2bin):
        log.debug(f'Not overriding mass cut for resonant bin: {q2bin}')
        return cuts

    cut_org = cuts['mass']
    cut_new = cut_org.replace('B_Mass', 'B_Mass_smr')
    cuts['mass'] = cut_new

    log.debug('Overriding:')
    log.debug(cut_org)
    log.debug('--->')
    log.debug(cut_new)

    return cuts
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
def apply_full_selection(
        rdf      : RDataFrame,
        q2bin    : str,
        process  : str,
        project  : str = None,
        analysis : str = None,
        trigger  : str = None) -> dict[str,str]:
    '''
    Will apply full selection on dataframe
    '''
    d_sel = selection(project=project, q2bin=q2bin, process=process, analysis=analysis, trigger=trigger)
    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    return rdf
#-----------------------
