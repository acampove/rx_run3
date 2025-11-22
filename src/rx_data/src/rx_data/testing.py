'''
This module contains functions needed by tests
'''

from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter
from ROOT                   import RDataFrame # type: ignore

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:testing')

l_trigger_ee  = [
    'Hlt2RD_BuToKpEE_MVA',
    'Hlt2RD_BuToKpEE_MVA_misid',
    'Hlt2RD_B0ToKpPimEE_MVA',
    'Hlt2RD_B0ToKpPimEE_MVA_misid']

l_prefix_kind = [
    ('Hlt2RD_BuToKpEE'     , 'mc_ee'),
    ('Hlt2RD_BuToKpEE'     , 'dt_ss'),
    ('Hlt2RD_BuToKpEE'     , 'dt_ee'),
    ('Hlt2RD_BuToKpEE'     , 'dt_mi'),
    # -----------
    ('Hlt2RD_B0ToKpPimEE'  , 'mc_ee'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_ss'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_ee'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_mi'),
    # -----------
    ('Hlt2RD_BuToKpMuMu'   , 'mc_mm'),
    ('Hlt2RD_BuToKpMuMu'   , 'dt_mm'),
    # -----------
    ('Hlt2RD_B0ToKpPimMuMu', 'mc_mm'),
    ('Hlt2RD_B0ToKpPimMuMu', 'dt_mm')]

l_prefix_kind_data = [
    ('Hlt2RD_BuToKpEE'     , 'dt_ss'),
    ('Hlt2RD_BuToKpEE'     , 'dt_ee'),
    ('Hlt2RD_BuToKpEE'     , 'dt_mi'),
    # -----------
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_ss'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_ee'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_mi'),
    # -----------
    ('Hlt2RD_BuToKpMuMu'   , 'dt_mm'),
    ('Hlt2RD_B0ToKpPimMuMu', 'dt_mm')]

l_prefix_kind_bplus = [
    ('Hlt2RD_BuToKpEE'     , 'mc_ee'),
    ('Hlt2RD_BuToKpEE'     , 'dt_ss'),
    ('Hlt2RD_BuToKpEE'     , 'dt_ee'),
    ('Hlt2RD_BuToKpEE'     , 'dt_mi'),
    # -----------
    ('Hlt2RD_BuToKpMuMu'   , 'mc_mm'),
    ('Hlt2RD_BuToKpMuMu'   , 'dt_mm')]

l_prefix_kind_bzero = [
    ('Hlt2RD_B0ToKpPimEE'  , 'mc_ee'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_ss'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_ee'),
    ('Hlt2RD_B0ToKpPimEE'  , 'dt_mi'),
    # -----------
    ('Hlt2RD_B0ToKpPimMuMu', 'mc_mm'),
    ('Hlt2RD_B0ToKpPimMuMu', 'dt_mm')]
# ----------------------------------
class Data:
    nentries= 100_000
# ----------------------------------
def get_rdf(kind : str, prefix : str) -> RDataFrame:
    '''
    Parameters
    ------------------
    kind  : Represents type of sample, dt_ss, dt_ee, dt_mi, dt_mm, mc_ee, mc_mm
    prefix: Trigger name substring, before _MVA

    Returns
    ------------------
    ROOT dataframe
    '''
    if   kind == 'dt_ss':
        sample = 'DATA_24_MagUp_24c3'
        trigger= f'{prefix}_SameSign_MVA'
    elif kind == 'dt_ee':
        sample = 'DATA_24_MagUp_24c2'
        trigger= f'{prefix}_MVA'
    elif kind == 'dt_mi':
        sample = 'DATA_24_MagUp_24c4'
        trigger= f'{prefix}_MVA_misid'
    elif kind == 'dt_mm':
        sample = 'DATA_24_MagDown_24c4'
        trigger= f'{prefix}_MVA'
    elif kind == 'mc_ee' and prefix.endswith('EE'):
        sample = 'Bd_Kstee_eq_btosllball05_DPC'
        trigger= f'{prefix}_MVA'
    elif kind == 'mc_mm' and prefix.endswith('MuMu'):
        sample = 'Bd_Kstmumu_eq_btosllball05_DPC'
        trigger= f'{prefix}_MVA'
    else:
        raise ValueError(f'Invalid dataset of kind/prefix: {kind}/{prefix}')

    return rdf_from_sample(sample=sample, trigger=trigger)
# ----------------------------------
def rdf_from_sample(sample : str, trigger : str) -> RDataFrame:
    with RDFGetter.max_entries(value = Data.nentries):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    rdf = _apply_selection(rdf=rdf, trigger=trigger, sample=sample)

    nentries = rdf.Count().GetValue()
    if nentries == 0:
        rep = rdf.Report()
        rep.Print()
        raise ValueError(f'No entry passed for {sample}/{trigger}')

    return rdf
# ----------------------------------
def _apply_selection(rdf : RDataFrame, trigger : str, sample : str) -> RDataFrame:
    d_sel = sel.selection(trigger=trigger, q2bin='jpsi', process=sample)
    d_sel['pid_l']      = '(1)'
    d_sel['jpsi_misid'] = '(1)'
    d_sel['cascade']    = '(1)'
    d_sel['hop']        = '(1)'

    for cut_name, cut_expr in d_sel.items():
        log.debug(f'{cut_name:<20}{cut_expr}')
        rdf = rdf.Filter(cut_expr, cut_name)

    rep   = rdf.Report()
    rep.Print()

    return rdf
# ----------------------
def get_trigger(kind : str, prefix : str) -> str:
    '''
    Parameters
    -------------
    kind: Signals sample type
    prefis: Start of the trigger name

    Returns
    -------------
    HLT2 trigger name
    '''
    if kind.endswith('_ss'):
        suffix = '_SameSign_MVA'
    elif kind.endswith('_mi'):
        suffix = '_MVA_misid'
    elif kind.endswith(('_ee', '_mm')):
        suffix = '_MVA'
    else:
        raise ValueError(f'Invalid kind: {kind}')

    return f'{prefix}{suffix}'
