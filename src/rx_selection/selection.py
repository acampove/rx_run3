'''
Module containing the selection function, which returns a dictionary of cuts
'''
# pylint: disable=too-many-positional-arguments, too-many-arguments

from dataclasses         import dataclass
from importlib.resources import files

import yaml
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
    l_ee_trig   = ['ETOS', 'HTOS', 'GTIS']
    l_mm_trig   = ['MTOS']
    l_year_r12  = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_year_run3 = ['2024']
    l_year      = l_year_r12 + l_year_run3
#-----------------------
def selection(kind : str, trigger : str, year : str, proc : str, q2bin: str, decay : str) -> dict[str,str]:
    '''
    Picks up sample name, trigger, etc, returns dictionary with selection

    Kind   : Kind of selection, e.g. all_gorder (legacy name, meaning all/full selection in Giulia's [former colleague] ordering)
    trigger: L0 trigger for Run12, ETOS or MTOS for Run3, used to pick channel
    proc   : Short name for sample, e.g. data, ctrl, etc, used to pick truth matching
    decay  : Decay targetted, e.g. bukee, bdkstee, etc, used to pick actual cuts
    '''
    if not year in Data.l_year:
        raise ValueError(f'Cannot find {year} among: {Data.l_year}')

    log.info(f'Using q2bin {q2bin} for cut type {kind}, process {proc}, trigger {trigger} and year {year}')

    d_cut : dict[str,str] = {}

    d_cut['truth'] = tm.get_truth(proc)

    if   decay in [  'bukee',   'bukmm'] and year in Data.l_year_r12:
        d_tmp = _get_bukll_run12_selection(kind, trigger, q2bin, year, decay)
    elif decay in [  'bukee',   'bukmm'] and year in Data.l_year_run3:
        d_tmp = _get_bukll_run3_selection(kind, trigger, q2bin, year, decay)
    elif decay in ['bdkstee', 'bdkstmm'] and year in Data.l_year_run3:
        d_tmp = _get_bdkst_run3_selection(kind, trigger, q2bin, year, decay)
    else:
        raise ValueError(f'Invalid decay: {decay}')

    d_cut.update(d_tmp)

    return d_cut
#-----------------------
def _get_bukll_run12_selection(kind: str, trigger : str, q2bin : str, year : str, decay : str) -> dict[str,str]:
    d_cut         = {}
    trigger_lower = trigger.lower()

    if   kind == 'all_gorder' and trigger in Data.l_ee_trig:
        l_cut = ['nspd', trigger_lower, 'hlt1', 'hlt2', 'q2', 'kinematics', 'cascade', 'ghost', 'calo_rich', 'pid', 'xyecal', 'bdt', 'mass']
    elif kind == 'all_gorder' and trigger in Data.l_mm_trig:
        l_cut = ['nspd', trigger_lower, 'hlt1', 'hlt2', 'q2', 'kinematics', 'cascade', 'ghost',      'rich', 'acceptance', 'pid', 'jpsi_misid', 'bdt', 'mass']
    else:
        raise ValueError(f'Wrong kind "{kind}" and/or trigger "{trigger}"')

    d_cut = { cut : rs.get(cut, trigger, q2bin, year, decay) for cut in l_cut }

    return d_cut
#-----------------------
def _get_bukll_run3_selection(kind : str, trigger : str, q2bin : str, year : str, decay : str) -> dict[str,str]:
    d_cut         = {}
    if   kind == 'all_gorder' and trigger in Data.l_ee_trig:
        l_cut = ['hlt1', 'hlt2', 'q2', 'kinematics', 'cascade', 'ghost', 'rich', 'calo', 'pid_l', 'pid_k', 'jpsi_misid', 'bdt', 'mass']
    elif kind == 'all_gorder' and trigger in Data.l_mm_trig:
        l_cut = ['hlt1', 'hlt2', 'q2', 'kinematics', 'cascade', 'ghost', 'rich', 'muon', 'pid_l', 'pid_k', 'jpsi_misid', 'bdt', 'mass']
    else:
        raise ValueError(f'Wrong kind "{kind}" and/or trigger "{trigger}"')

    d_cut = { cut : rs.get(cut, trigger, q2bin, year, decay) for cut in l_cut }

    return d_cut
#-----------------------
def _get_bdkst_run3_selection(kind : str, trigger : str, q2bin : str, year :str , decay : str) -> dict[str,str]:
    d_cut         = {}
    if kind == 'all_gorder':
        l_cut = [
                'hlt1',
                'hlt2',
                'q2',
                'kinem_lep',
                'kinem_had',
                'kst_mass',
                'kst_pt',
                'cascade',
                'ghost',
                'rich',
                'acceptance',
                'pid_l',
                'pid_k',
                'pid_pi',
                'jpsi_misid',
                'bdt',
                'mass',
                ]
    else:
        raise ValueError(f'Wrong kind "{kind}"')

    d_cut = { cut : rs.get(cut, trigger, q2bin, year, decay) for cut in l_cut }

    return d_cut
