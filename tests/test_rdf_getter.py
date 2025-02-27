'''
Class testing RDFGetter
'''
import os
import matplotlib.pyplot as plt

import pytest
import ROOT
from ROOT                   import RDataFrame, EnableImplicitMT
from dmu.logging.log_store  import LogStore
from rx_data.rdf_getter     import RDFGetter

# ------------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    EnableImplicitMT(10)

    out_dir    = '/tmp/rx_data/tests/rdf_getter'
    low_q2     = '(Jpsi_M * Jpsi_M >        0) && (Jpsi_M * Jpsi_M <  1000000)'
    central_q2 = '(Jpsi_M * Jpsi_M >  1100000) && (Jpsi_M * Jpsi_M <  6000000)'
    jpsi_q2    = '(Jpsi_M * Jpsi_M >  6000000) && (Jpsi_M * Jpsi_M < 12960000)'
    psi2_q2    = '(Jpsi_M * Jpsi_M >  9920000) && (Jpsi_M * Jpsi_M < 16400000)'
    high_q2    = '(Jpsi_M * Jpsi_M > 15500000) && (Jpsi_M * Jpsi_M < 22000000)'

    l_branch = ['mva.mva_cmb', 'mva.mva_prc', 'hop.hop_mass', 'hop.hop_alpha']
# ------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter', 10)
    os.makedirs(Data.out_dir, exist_ok=True)

    RDFGetter.samples = {
            'main'      : '/home/acampove/external_ssd/Data/samples/main.yaml',
            'mva'       : '/home/acampove/external_ssd/Data/samples/mva.yaml',
            'hop'       : '/home/acampove/external_ssd/Data/samples/hop.yaml',
            'cascade'   : '/home/acampove/external_ssd/Data/samples/cascade.yaml',
            'jpsi_misid': '/home/acampove/external_ssd/Data/samples/jpsi_misid.yaml',
            }
# ------------------------------------------------
def _check_branches(rdf : RDataFrame) -> None:
    l_name = [ name.c_str() for name in rdf.GetColumnNames() ]

    for branch in Data.l_branch:
        if branch in l_name:
            continue

        raise ValueError(f'Branch missing: {branch}')
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
@pytest.mark.parametrize('sample', ['DATA_24_MagUp_24c1', 'DATA_24_MagUp_24c2', 'DATA_24_Mag*_24c*'])
def test_data(sample : str):
    '''
    Test of getter class in data
    '''
    gtr = RDFGetter(sample=sample, trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()

    _check_branches(rdf)

    sample = sample.replace('*', 'p')
    _plot_mva_mass(rdf, sample)
    _plot_mva(rdf, sample)
    _plot_hop(rdf, sample)
# ------------------------------------------------
@pytest.mark.parametrize('sample', ['Bu_JpsiPi_ee_eq_DPC'])
#@pytest.mark.parametrize('sample', ['Bu_JpsiK_ee_eq_DPC'])
def test_mc(sample : str):
    '''
    Test of getter class in mc
    '''

    gtr = RDFGetter(sample=sample, trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()

    _check_branches(rdf)
    _plot_mva_mass(rdf, sample)
    _plot_mva(rdf     , sample)
    _plot_hop(rdf     , sample)
# ------------------------------------------------
