'''
Module with tests for swap calculator class
'''
import os

import mplhep
import pytest
import matplotlib.pyplot as plt
from ROOT                   import RDataFrame # type: ignore
from pathlib                import Path
from dmu.logging.log_store  import LogStore
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter
from rx_data.swp_calculator import SWPCalculator

log = LogStore.add_logger('rx_data:test_swp_calculator')
# ----------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    nentries= 100_000
    user    = os.environ['USER']
    out_dir = Path(f'/tmp/{user}/tests/rx_data/swap_calculator')
# ----------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    os.makedirs(Data.out_dir, exist_ok=True)
    LogStore.set_level('rx_data:swp_calculator'     , 20)
    LogStore.set_level('rx_data:test_swp_calculator', 10)
    LogStore.set_level('rx_data:rdf_getter'         , 10)

    plt.style.use(mplhep.style.LHCb2)
# ----------------------------------
def _get_rdf(kind : str, prefix : str) -> RDataFrame:
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

    with RDFGetter.max_entries(value = Data.nentries):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    rdf = _apply_selection(rdf, trigger, sample)

    nentries = rdf.Count().GetValue()
    if nentries == 0:
        rep = rdf.Report()
        rep.Print()
        raise ValueError(f'No entry passed for {kind}/{trigger}')

    return rdf
# ----------------------------------
def _apply_selection(rdf : RDataFrame, trigger : str, sample : str) -> RDataFrame:
    d_sel = sel.selection(trigger=trigger, q2bin='jpsi', process=sample)
    d_sel['pid_l']      = '(1)'
    d_sel['jpsi_misid'] = '(1)'
    d_sel['cascade']    = '(1)'
    d_sel['hop']        = '(1)'
    d_sel['bdt']        = 'mva_cmb > 0.9'

    for cut_name, cut_expr in d_sel.items():
        log.debug(f'{cut_name:<20}{cut_expr}')
        rdf = rdf.Filter(cut_expr, cut_name)

    rep   = rdf.Report()
    rep.Print()

    return rdf
# ----------------------
def _get_hadron_mapping(prefix : str) -> dict[str,int]:
    '''
    Parameters
    -------------
    preffix: Used to distinguish between triggers

    Returns
    -------------
    dictionary specifying how to swap masses
    '''
    if prefix in ['Hlt2RD_BuToKpEE', 'Hlt2RD_BuToKpMuMu']:
        return {'H' : 321}

    if prefix in ['Hlt2RD_B0ToKpPimEE', 'Hlt2RD_B0ToKpPimMuMu']:
        return {'H1' : 321}

    raise ValueError(f'Invalid prefix: {prefix}')
# ----------------------------------
@pytest.mark.parametrize('prefix, kind',
    [
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
    ('Hlt2RD_B0ToKpPimMuMu', 'dt_mm')])
def test_dzero_misid(prefix : str, kind : str):
    '''
    Tests dzero decay contamination
    '''
    rdf = _get_rdf(kind=kind, prefix=prefix)
    ientries = rdf.Count().GetValue()

    d_had = _get_hadron_mapping(prefix=prefix)

    obj = SWPCalculator(rdf, d_lep={'L1' : 211, 'L2' : 211}, d_had=d_had)
    rdf = obj.get_rdf(preffix='dzero_misid', progress_bar=True, use_ss= 'ss' in kind)

    oentries = rdf.Count().GetValue()

    assert ientries == oentries

    _plot(rdf, test='dzero_misid', kind=kind, prefix=prefix)
# ----------------------------------
@pytest.mark.parametrize('prefix, kind',
    [
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
    ('Hlt2RD_B0ToKpPimMuMu', 'dt_mm')])
def test_phi_misid(prefix : str, kind : str):
    '''
    Tests phi decay contamination
    '''
    rdf   = _get_rdf(kind=kind, prefix=prefix)

    if   prefix in ['Hlt2RD_BuToKpEE', 'Hlt2RD_BuToKpMuMu']:
        obj = SWPCalculator(rdf, d_lep={'L1' : 321, 'L2' : 321}, d_had={'H' : 321})
    elif prefix in ['Hlt2RD_B0ToKpPimEE', 'Hlt2RD_B0ToKpPimMuMu']:
        obj = SWPCalculator(rdf, d_lep={'H1' : 321}, d_had={'H2' : 321})
    else:
        raise ValueError(f'Invalid prefix: {prefix}')

    rdf = obj.get_rdf(preffix='phi_misid', progress_bar=True, use_ss= 'ss' in kind)

    _plot(rdf, test='phi_misid', kind=kind, prefix=prefix)
# ----------------------------------
@pytest.mark.parametrize('prefix, kind',
    [
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
    ('Hlt2RD_B0ToKpPimMuMu', 'dt_mm')])
def test_jpsi_misid(prefix : str, kind : str):
    '''
    Tests jpsi misid contamination
    '''
    rdf      = _get_rdf(kind=kind, prefix=prefix)
    had_name = 'H' if prefix in ['Hlt2RD_BuToKpMuMu', 'Hlt2RD_BuToKpEE'] else 'H1'

    if 'MuMu' in prefix:
        obj = SWPCalculator(rdf, d_lep={'L1' : 13, 'L2' : 13}, d_had={had_name : 13})
    elif 'EE' in prefix:
        obj = SWPCalculator(rdf, d_lep={'L1' : 11, 'L2' : 11}, d_had={had_name : 11})
    else:
        raise ValueError(f'Invalid prefix: {prefix}')

    rdf = obj.get_rdf(preffix='jpsi_misid', progress_bar=True, use_ss= 'ss' in kind)

    _plot(rdf, test='jpsi_misid', kind=kind, prefix=prefix)
# ----------------------------------
def _plot(rdf : RDataFrame, test : str, kind : str, prefix : str):
    d_data = rdf.AsNumpy([f'{test}_mass_swp', f'{test}_mass_org'])
    arr_swp= d_data[f'{test}_mass_swp']
    arr_org= d_data[f'{test}_mass_org']

    mass_rng = {
        'jpsi_misid' : (2700, 3300),
        'dzero_misid': (1700, 2000),
        'phi_misid'  : ( 980, 1100)}[test]

    plt.hist(arr_org, bins=80, range=mass_rng, alpha=0.5, label='Original', color='gray')
    plt.hist(arr_swp, bins=80, range=mass_rng, histtype='step', label='Swapped', color='blue')
    plt.grid(False)

    if test == 'phi_misid':
        plt.axvline(x=1020, color='r', label=r'$\phi$', linestyle=':')
    elif test == 'jpsi_misid':
        plt.axvline(x=3100, color='r', label=r'$J/\psi$', linestyle=':')
    else:
        plt.axvline(x=1864, color='r', label='$D_0$', linestyle=':')

    out_dir = Data.out_dir/test
    out_dir.mkdir(parents=True, exist_ok=True)

    plt.legend()
    plt.savefig(out_dir/f'{kind}_{prefix}.png')
    plt.close('all')
# ----------------------------------
