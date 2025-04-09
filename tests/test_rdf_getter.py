'''
Class testing RDFGetter
'''
import os
import matplotlib.pyplot as plt

import pytest
import ROOT
from ROOT                   import RDataFrame, EnableImplicitMT
from dmu.logging.log_store  import LogStore
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter

log=LogStore.add_logger('rx_data:test_rdf_getter')
# ------------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    EnableImplicitMT(10)

    out_dir    = '/tmp/tests/rx_data/rdf_getter'
    low_q2     = '(Jpsi_M * Jpsi_M >        0) && (Jpsi_M * Jpsi_M <  1000000)'
    central_q2 = '(Jpsi_M * Jpsi_M >  1100000) && (Jpsi_M * Jpsi_M <  6000000)'
    jpsi_q2    = '(Jpsi_M * Jpsi_M >  6000000) && (Jpsi_M * Jpsi_M < 12960000)'
    psi2_q2    = '(Jpsi_M * Jpsi_M >  9920000) && (Jpsi_M * Jpsi_M < 16400000)'
    high_q2    = '(Jpsi_M * Jpsi_M > 15500000) && (Jpsi_M * Jpsi_M < 22000000)'

    l_branch_ee = [
            'brem_track_2.B_M_brem_track_2',
            'mva.mva_cmb',
            'mva.mva_prc',
            'hop.hop_mass',
            'hop.hop_alpha',
            'cascade.swp_cascade_mass_swp',
            'jpsi_misid.swp_jpsi_misid_mass_swp',
            ]

    l_branch_mm = [
            'mva.mva_cmb',
            'mva.mva_prc',
            'hop.hop_mass',
            'hop.hop_alpha',
            'cascade.swp_cascade_mass_swp',
            'jpsi_misid.swp_jpsi_misid_mass_swp',
            ]
# ------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter', 10)
    os.makedirs(Data.out_dir, exist_ok=True)
# ------------------------------------------------
def _check_branches(rdf : RDataFrame, is_ee : bool) -> None:
    l_name = [ name.c_str() for name in rdf.GetColumnNames() ]

    l_branch = Data.l_branch_ee if is_ee else Data.l_branch_mm
    for branch in l_branch:
        if branch in l_name:
            continue

        raise ValueError(f'Branch missing: {branch}')
# ------------------------------------------------
def _print_dotted_branches(rdf : RDataFrame) -> None:
    l_name = [ name.c_str() for name in rdf.GetColumnNames() ]

    for name in l_name:
        if '.' not in name:
            continue

        log.info(name)
# ------------------------------------------------
def _plot_mva_mass(rdf : RDataFrame, test : str) -> None:
    test_dir = f'{Data.out_dir}/{test}'
    os.makedirs(test_dir, exist_ok=True)

    rdf = rdf.Filter(Data.jpsi_q2)

    for cmb in [0.4, 0.6, 0.8, 0.9]:
        rdf      = rdf.Filter(f'mva_cmb > {cmb}')
        arr_mass = rdf.AsNumpy(['B_M'])['B_M']
        plt.hist(arr_mass, bins=50, histtype='step', range=[4800, 5500], label=f'{cmb}; 0.0')

    for prc in [0.5, 0.6]:
        rdf      = rdf.Filter(f'mva_prc > {prc}')
        arr_mass = rdf.AsNumpy(['B_M'])['B_M']
        plt.hist(arr_mass, bins=50, histtype='step', range=[4800, 5500], label=f'{cmb}; {prc}')

    plt.title(test)
    plt.legend()
    plt.savefig(f'{test_dir}/mva_mass.png')
    plt.close()
# ------------------------------------------------
def _plot_q2_track(rdf : RDataFrame, sample : str) -> None:
    test_dir = f'{Data.out_dir}/{sample}'
    os.makedirs(test_dir, exist_ok=True)

    arr_q2_track = rdf.AsNumpy(['q2_track'])['q2_track']
    arr_q2       = rdf.AsNumpy(['q2'      ])['q2'      ]

    plt.hist(arr_q2_track, alpha=0.5      , range=[0, 22_000_000], bins=40, label='$q^2_{track}$')
    plt.hist(arr_q2      , histtype='step', range=[0, 22_000_000], bins=40, label='$q^2$')

    plt.title(sample)
    plt.legend()
    plt.savefig(f'{test_dir}/q2_track.png')
    plt.close()
# ------------------------------------------------
def _plot_mva(rdf : RDataFrame, test : str) -> None:
    test_dir = f'{Data.out_dir}/{test}'
    os.makedirs(test_dir, exist_ok=True)

    rdf = rdf.Filter(Data.jpsi_q2)

    arr_cmb = rdf.AsNumpy(['mva_cmb'])['mva_cmb']
    arr_prc = rdf.AsNumpy(['mva_prc'])['mva_prc']
    plt.hist(arr_cmb, bins=40, histtype='step', range=[0, 1], label='CMB')
    plt.hist(arr_prc, bins=40, histtype='step', range=[0, 1], label='PRC')

    plt.title(test)
    plt.legend()
    plt.savefig(f'{test_dir}/mva.png')
    plt.close()
# ------------------------------------------------
def _plot_hop(rdf : RDataFrame, test : str) -> None:
    test_dir = f'{Data.out_dir}/{test}'
    os.makedirs(test_dir, exist_ok=True)

    rdf = rdf.Filter(Data.jpsi_q2)

    arr_org = rdf.AsNumpy(['B_M' ])['B_M' ]
    arr_hop = rdf.AsNumpy(['hop_mass'])['hop_mass']
    plt.hist(arr_org, bins=80, histtype='step', range=[3000, 7000], label='Original')
    plt.hist(arr_hop, bins=80, histtype='step', range=[3000, 7000], label='HOP')
    plt.title(test)
    plt.legend()
    plt.savefig(f'{test_dir}/hop_mass.png')
    plt.close()

    arr_aph = rdf.AsNumpy(['hop_alpha'])['hop_alpha']
    plt.hist(arr_aph, bins=40, histtype='step', range=[0, 5])
    plt.title(test)
    plt.savefig(f'{test_dir}/hop_alpha.png')
    plt.close()
# ------------------------------------------------
def _apply_selection(rdf : RDataFrame, trigger : str, sample : str, override : dict[str,str] = None) -> RDataFrame:
    d_sel = sel.selection(project='RK', trigger=trigger, q2bin='jpsi', process=sample)
    if override is not None:
        d_sel.update(override)

    for cut_name, cut_expr in d_sel.items():
        if cut_name in ['mass', 'q2']:
            continue
        rdf = rdf.Filter(cut_expr, cut_name)

    return rdf
# ------------------------------------------------
def _plot_brem_track_2(rdf : RDataFrame, test : str, tree : str) -> None:
    test_dir = f'{Data.out_dir}/{test}/{tree}'
    os.makedirs(test_dir, exist_ok=True)

    d_var= {
            'B_M'             : [4200,  6000],
            'Jpsi_M'          : [2500,  3300],
            'L1_PT'           : [   0, 10000],
            'L2_PT'           : [   0, 10000],
            'L1_HASBREMADDED' : [0, 2],
            'L2_HASBREMADDED' : [0, 2],
            }

    kind = 'brem_track_2'
    for var, rng in d_var.items():
        name = f'{kind}.{var}_{kind}'
        arr_org = rdf.AsNumpy([var ])[var ]
        arr_cor = rdf.AsNumpy([name])[name]

        plt.hist(arr_org, bins=50, alpha=0.5      , range=rng, label='Original' , color='gray')
        plt.hist(arr_cor, bins=50, histtype='step', range=rng, label='Corrected', color='blue')

        plt.title(f'{var}; {test}')
        plt.legend()
        plt.savefig(f'{test_dir}/{var}.png')
        plt.close()
# ------------------------------------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_BuToKpMuMu_MVA' ])
def test_data(sample : str, trigger : str):
    '''
    Test of getter class in data
    '''
    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_selection(rdf=rdf, trigger=trigger, sample=sample)
    rep = rdf.Report()
    rep.Print()

    _check_branches(rdf, is_ee = 'MuMu' not in trigger)

    sample = sample.replace('*', 'p')

    _plot_mva_mass(rdf, sample)
    _plot_mva(rdf, sample)
    _plot_hop(rdf, sample)
# ------------------------------------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2', 'Bu_JpsiK_ee_eq_DPC', 'Bu_psi2SK_ee_eq_DPC', 'Bu_JpsiX_ee_eq_JpsiInAcc'])
def test_q2_track_electron(sample : str):
    '''
    Checks the distributions of q2_track vs normal q2
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'

    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_selection(rdf=rdf, trigger=trigger, sample=sample)
    rep = rdf.Report()
    rep.Print()

    _check_branches(rdf, is_ee=True)

    sample = sample.replace('*', 'p')

    _plot_q2_track(rdf, sample)
# ------------------------------------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2', 'Bu_Kmumu_eq_btosllball05_DPC']) 
def test_q2_track_muon(sample : str):
    '''
    Checks the distributions of q2_track vs normal q2
    '''
    trigger = 'Hlt2RD_BuToKpMuMu_MVA'

    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_selection(rdf=rdf, trigger=trigger, sample=sample)
    rep = rdf.Report()
    rep.Print()

    _check_branches(rdf, is_ee=False)

    sample     = sample.replace('*', 'p')
    identifier = f'{trigger}_{sample}'

    _plot_q2_track(rdf, identifier)
# ------------------------------------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2', 'Bu_JpsiK_ee_eq_DPC'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA'])
def test_brem_track_2(sample : str, trigger : str):
    '''
    Test brem_track_2 correction
    '''
    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_selection(rdf=rdf, trigger=trigger, override = {'mass' : 'B_const_mass_M > 5160'}, sample=sample)
    rep = rdf.Report()
    rep.Print()

    _check_branches(rdf, is_ee=True)

    sample = sample.replace('*', 'p')
    _plot_brem_track_2(rdf, sample, 'brem_track_2')
# ------------------------------------------------
@pytest.mark.parametrize('sample', ['Bu_JpsiK_ee_eq_DPC'])
def test_mc(sample : str):
    '''
    Test of getter class in mc
    '''

    gtr = RDFGetter(sample=sample, trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()

    _check_branches(rdf, is_ee=True)
    _plot_mva_mass(rdf, sample)
    _plot_mva(rdf     , sample)
    _plot_hop(rdf     , sample)
# ------------------------------------------------
@pytest.mark.parametrize('sample', ['Bu_JpsiK_ee_eq_DPC', 'DATA_24_MagDown_24c2'])
def test_check_vars(sample : str):
    '''
    Checks that variables from friend trees can be accessed
    '''
    gtr = RDFGetter(sample=sample, trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()

    _check_branches(rdf, is_ee=True)
    _print_dotted_branches(rdf)
# ------------------------------------------------
@pytest.mark.parametrize('sample', ['Bu_JpsiK_ee_eq_DPC'])
def test_mcdecaytree(sample : str):
    '''
    Builds dataframe from MCDecayTree
    '''
    gtr = RDFGetter(sample=sample, trigger='Hlt2RD_BuToKpEE_MVA', tree='MCDecayTree')
    rdf = gtr.get_rdf()

    nentries = rdf.Count().GetValue()

    log.info(f'Found {nentries} entries')

    assert nentries > 0
# ------------------------------------------------
