'''
Module containing the selection function, which returns a dictionary of cuts
'''
# pylint: disable=too-many-positional-arguments, too-many-arguments, import-error

import os
import re

from importlib.resources import files
from contextlib          import contextmanager

import yaml
import ap_utilities.decays.utilities as aput
from ROOT                   import RDataFrame
from dmu.generic            import hashing
from dmu.logging.log_store  import LogStore

from rx_data      import utilities          as dut
from rx_selection import truth_matching     as tm
from rx_selection import version_management as vman

log=LogStore.add_logger('rx_selection:selection')
#-----------------------
class Data:
    '''
    Class used to store share attributes
    '''
    l_project  = ['RK', 'RKst']
    l_analysis = ['EE', 'MM'  ]
    l_q2bin    = ['low', 'central', 'jpsi', 'psi2S', 'high']

    d_custom_selection : dict[str,str]|None = None
#-----------------------
class MultipleSelectionOverriding(Exception):
    '''
    Will be risen when global selection is overriden more than once per run
    '''
    def __init__(self, message):
        super().__init__(message)
#-----------------------
@contextmanager
def custom_selection(d_sel : dict[str,str]):
    '''
    This is a context manager meant to be used to add/overide
    the selection specified in `d_sel` to default selection.

    Parameters
    ------------------
    d_sel: Dictionary with
        - key : Name of cut, e.g. brem
        - value: Definition of cut, e.g. nbrem > 0
    '''
    org_val = Data.d_custom_selection

    try:
        Data.d_custom_selection = d_sel
        yield
    finally:
        Data.d_custom_selection = org_val
#-----------------------
def _print_selection(d_cut : dict[str,str]) -> None:
    for name, expr in d_cut.items():
        log.debug(f'{name:<20}{expr}')
#-----------------------
def set_custom_selection(d_cut : dict[str,str]) -> None:
    '''
    This function is meant to override the analysis selection, such that

    - Every tool that uses the selection picks up the same selection
    - Cuts are added (e.g. brem category) or modified (e.g. MVA WP)

    This function is meant to be called once, at the beginning of the process, e.g. main function.
    '''
    if Data.d_custom_selection is not None:
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

    if Data.d_custom_selection is None:
        log.warning('No custom selection found')
        return

    log.warning('Resetting custom selection')

    Data.d_custom_selection = None
#-----------------------
def selection(
        q2bin    : str,
        process  : str,
        trigger  : str,
        smeared  : bool= True) -> dict[str,str]:
    '''
    Picks up sample name, trigger, etc, returns dictionary with selection

    q2bin    : low, central, jpsi, psi2S or high
    process  : Nickname for MC sample, starts with "DATA" for data
    smeared  : If true (default), the selection will use cuts on smeared masses. Only makes sense for electron MC samples
    trigger  : E.g. Hlt2RD...
    '''
    if 'toy' in process:
        log.warning(f'Process {process} recognized as toy sample, returning empty selection')
        return {}

    project  = dut.project_from_trigger(trigger=trigger)
    analysis = dut.channel_from_trigger(trigger=trigger)

    d_cut : dict[str,str] = {}

    event_type     = process if process.startswith('DATA') else aput.read_event_type(nickname=process)
    log.info(f'{process:<40}{"->":20}{event_type:<20}')

    if process.startswith('DATA'):
        log.debug('Adding cleaning requirement for data')
        d_cut['clean'] = 'dataq == 1'
        d_cut['block'] = 'block >= 1'
    else:
        log.debug('Adding truth matching requirement for MC')
        d_cut['truth'] = tm.get_truth(event_type)

    d_tmp = _get_selection(analysis, project, q2bin)
    d_cut.update(d_tmp)

    if Data.d_custom_selection is not None:
        d_cut.update(Data.d_custom_selection)

    d_cut = _update_mass_cuts(
        d_cut   =   d_cut,
        q2bin   =   q2bin,
        sample  = process,
        trigger = trigger,
        smeared = smeared)

    d_cut_final = {}
    for cut_name, cut_expr in d_cut.items():
        d_cut_final[cut_name] = _override_block(cut_block = cut_expr, process = process)

    _print_selection(d_cut_final)

    return d_cut_final
#-----------------------
def _update_mass_cuts(
        d_cut   : dict[str,str],
        q2bin   : str,
        sample  : str,
        trigger : str,
        smeared : bool) -> dict[str,str]:

    should_smear = dut.is_mc(sample=sample) and dut.is_ee(trigger=trigger)

    if not should_smear:
        log.debug(f'Not using cuts on smeared masses for {sample}/{trigger}')
        return d_cut

    if not smeared:
        log.warning('Using cuts on un-smeared masses')
        return d_cut

    log.debug('Using cuts on smeared masses')
    d_cut = _use_smeared_masses(cuts=d_cut, q2bin=q2bin)

    return d_cut
#-----------------------
def _use_smeared_masses(cuts : dict[str,str], q2bin : str) -> dict[str,str]:
    log.info('Overriding selection for electron MC to use smeared q2 and mass')

    cut_org = cuts['q2']
    if 'q2_track' not in cut_org:
        cut_new    = cut_org.replace('q2', 'q2_smr')
        cuts['q2'] = cut_new

        log.debug('Overriding:')
        log.debug(cut_org)
        log.debug('--->')
        log.debug(cut_new)
    else:
        # TODO: IF we use the q2_track for this cut, we need to find the
        # correct smearing factors here
        log.warning(f'Not overriding with smeared version q2 cut: {cut_org}')

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
    '''
    Parameters
    -----------------
    chan  : Channel, e.g. EE, MM
    proj  : Project, e.g. RK, RKst
    q2_bin: q2 bin e.g. central, needed to pick for q2 dependent cuts

    Returns 
    -----------------
    Dictionary with:

    key  : Label of cut, e.g. q2
    value: Cut expression e.g. q2 > 1e6
    '''
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
            raise ValueError(f'Cannot find q2 bin {q2_bin} in {cut_name} section')

        cut_val = d_q2bin[q2_bin]
        log.debug(f'Overriding {cut_name} for {q2_bin}')

        d_new[cut_name] = cut_val

    return d_new
#-----------------------
# TODO: This needs to be removed once block 3 and 4 MC be available
def _override_block(
    cut_block : str,
    process   : str) -> str:
    '''
    Parameters
    ---------------
    cut_block: One of the cuts associated to the selection. 
               This method targets cuts of the form block == 3
               which have to be converted into block == 2

    process  : Sample used with this cut, e.g. DATA_24...

    Returns
    ---------------
    Either:

    Original cut: If this sample is not a simulated sample from blocks 3 or 4, which are missing
    Modified cut: Otherwise
    '''

    if process.startswith('DATA'):
        log.debug(f'Not redefining block cut for {process}')
        return cut_block

    if 'block' not in cut_block:
        log.debug(f'Block not found in: {cut_block}')
        return cut_block

    regex = r'block\s*==\s*(\d)'

    mtch  = re.match(regex, cut_block)
    if not mtch:
        raise ValueError(f'Cannot match {regex} to {cut_block}')

    block = mtch.group(1)
    if block not in ['3', '4']:
        return cut_block

    new_cut = f'block == {2}'

    log.warning(f'For sample {process} replacing:')
    log.warning(f'{cut_block:<20}{"--->":<20}{new_cut:<20}')

    return new_cut
#-----------------------
def apply_full_selection(
    rdf      : RDataFrame,
    q2bin    : str,
    process  : str,
    trigger  : str,
    ext_cut  : str|None = None,
    uid      : str|None = None) -> RDataFrame:
    '''
    Will apply full selection on dataframe.
    IMPORTANT: This HAS to be done lazily or else the rest of the code will be slowed down.

    Parameters
    --------------------
    uid: Unique identifier, used for hashing. If not passed no hashing will be done
    ext_cut: Extra cut, optional

    Returns
    --------------------
    Dataframe after full selection.
    If uid was passed, the uid will be recalculated and attached to the dataframe.
    '''

    d_sel = selection(q2bin=q2bin, process=process, trigger=trigger)
    if ext_cut is not None:
        d_sel['extra'] = ext_cut

    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    if uid is None:
        return rdf

    rdf.uid = hashing.hash_object([uid, d_sel])

    return rdf
#-----------------------
