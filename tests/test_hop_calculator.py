'''
Module containing tests for HOPVarAdder
'''
import os

import yaml
import pytest
import matplotlib.pyplot as plt
from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_data.hop_calculator import HOPCalculator
from rx_data.mis_calculator import MisCalculator

# ----------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir = '/tmp/rx_data/tests/hop_calculator'
# ----------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:hop_calculator', 10)
# ----------------------------
def _get_samples() -> dict:
    samples_list='/publicfs/ucas/user/campoverde/Data/RX_run3/v5/rx_samples.yaml'
    with open(samples_list, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# ----------------------------
def _get_rdf(sample : str, trigger : str) -> RDataFrame:
    cfg    = _get_samples()
    l_path = cfg[sample][trigger]
    rdf    = RDataFrame('DecayTree', l_path[0])
    rdf    = rdf.Range(10_000)

    mcl    = MisCalculator(rdf=rdf, trigger=trigger)
    rdf    = mcl.get_rdf()

    return rdf
# ----------------------------
def _plot_variables(rdf : RDataFrame, rdf_hop : RDataFrame, name : str) -> None:
    out_dir = f'{Data.out_dir}/{name}'
    os.makedirs(out_dir, exist_ok=True)

    d_data = rdf_hop.AsNumpy(['hop_alpha', 'hop_mass'])
    arr_ms = rdf.AsNumpy(['B_M'])['B_M']

    plt.hist(d_data['hop_alpha'], bins=40, range=[0,5])
    plt.savefig(f'{out_dir}/alpha.png')
    plt.close()

    plt.hist(d_data['hop_mass'], bins=40, histtype='step', label='HOP', range=[0, 10_000])
    plt.hist(            arr_ms, bins=40, histtype='step', label='Original')
    plt.legend()
    plt.title(name)
    plt.savefig(f'{out_dir}/mass.png')
    plt.close()
# ----------------------------
def _compare_sig_bkg(rdf_sig : RDataFrame, rdf_bkg : RDataFrame, name : str) -> None:
    out_dir = f'{Data.out_dir}/{name}'
    os.makedirs(out_dir, exist_ok=True)

    arr_sig = rdf_sig.AsNumpy(['hop_mass'])['hop_mass']
    arr_bkg = rdf_bkg.AsNumpy(['hop_mass'])['hop_mass']

    plt.hist(arr_sig, range=[3000, 6000], bins=50, histtype='step', density=True, label='Signal')
    plt.hist(arr_bkg, range=[3000, 6000], bins=50, histtype='step', density=True, label='Background')

    plt.legend()
    plt.savefig(f'{out_dir}/mass.png')
    plt.close()
# ----------------------------
def _get_hop(sample : str, trigger : str) -> tuple[RDataFrame, RDataFrame]:
    rdf     = _get_rdf(sample = sample, trigger=trigger)
    obj     = HOPCalculator(rdf=rdf)
    rdf_hop = obj.get_rdf(preffix='hop')

    return rdf_hop, rdf
# ----------------------------
@pytest.mark.parametrize('sample, trigger',
                         [('Bd_Kstmumu_eq_btosllball05_DPC' , 'Hlt2RD_BuToKpMuMu_MVA'),
                          ('Bu_Kmumu_eq_btosllball05_DPC'   , 'Hlt2RD_BuToKpMuMu_MVA'),
                          ('Bd_Kstee_eq_btosllball05_DPC'   , 'Hlt2RD_BuToKpEE_MVA'  ),
                          ('Bu_Kee_eq_btosllball05_DPC'     , 'Hlt2RD_BuToKpEE_MVA'  )])
def test_mc(sample : str, trigger : str):
    '''
    Test on MC
    '''
    rdf_hop, rdf_org = _get_hop(sample = sample, trigger=trigger)

    _plot_variables(rdf=rdf_org, rdf_hop=rdf_hop, name=f'mc_{sample}_{trigger}')
# ----------------------------
def test_compare():
    '''
    Compare signal with background
    '''
    trigger    = 'Hlt2RD_BuToKpEE_MVA'
    rdf_bkg, _ = _get_hop(sample = 'Bd_Kstee_eq_btosllball05_DPC', trigger=trigger)
    rdf_sig, _ = _get_hop(sample = 'Bu_Kee_eq_btosllball05_DPC'  , trigger=trigger)

    _compare_sig_bkg(rdf_sig, rdf_bkg, 'compare')
# ----------------------------
def test_extra_branches():
    '''
    Testing adding extra branches to RDF
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    rdf     = _get_rdf(sample = 'Bu_Kee_eq_btosllball05_DPC', trigger=trigger)

    obj     = HOPCalculator(rdf=rdf)
    rdf_hop = obj.get_rdf(preffix='hop')
    l_col   = [ name.c_str() for name in rdf_hop.GetColumnNames() ]

    assert 'EVENTNUMBER' in l_col
    assert 'RUNNUMBER'   in l_col
# ----------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_BuToKpMuMu_MVA'])
@pytest.mark.parametrize('sample', ['DATA_24_MagDown_24c1'])
def test_data(sample : str, trigger : str):
    '''
    Test with data
    '''
    rdf_hop, rdf_org = _get_hop(sample = sample, trigger=trigger)

    _plot_variables(rdf=rdf_org, rdf_hop=rdf_hop, name=f'data_{sample}_{trigger}')
# ----------------------------
