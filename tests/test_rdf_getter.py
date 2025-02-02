'''
Class testing RDFGetter
'''
import os
import matplotlib.pyplot as plt

import pytest
import ROOT
from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_data.rdf_getter     import RDFGetter

# ------------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir = '/tmp/rx_data/tests/rdf_getter'
# ------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter', 10)
    os.makedirs(Data.out_dir, exist_ok=True)
# ------------------------------------------------
def _check_branches(rdf : RDataFrame) -> None:
    l_name = [ name.c_str() for name in rdf.GetColumnNames() ]

    l_mva  = [ name for name in l_name if 'mva'  in name ]
    l_main = [ name for name in l_name if 'B_PT' in name ]

    print(l_mva)
    print(l_main)

    if 'mva.mva_cmb' not in l_name or 'B_PT' not in l_name:
        print(l_mva )
        print(l_main)
        raise ValueError('MVA branch missing')
# ------------------------------------------------
def _plot_mva_mass(rdf : RDataFrame, test : str) -> None:
    rdf = rdf.Filter('Jpsi_M > 2800 && Jpsi_M < 3200')

    for mva in [0.0, 0.2, 0.4, 0.6, 0.8]:
        rdf      = rdf.Filter(f'mva.mva_cmb > {mva}')
        arr_mass = rdf.AsNumpy(['B_M'])['B_M']

        plt.hist(arr_mass, bins=40, histtype='step', range=[5000, 5600], label=str(mva))

    plt.legend()
    plt.savefig(f'{Data.out_dir}/{test}_mva_mass.png')
    plt.close()
# ------------------------------------------------
def _plot_mva(rdf : RDataFrame, test : str) -> None:
    rdf = rdf.Filter('Jpsi_M > 2800 && Jpsi_M < 3200')

    arr_cmb = rdf.AsNumpy(['mva.mva_cmb'])['mva.mva_cmb']
    arr_prc = rdf.AsNumpy(['mva.mva_prc'])['mva.mva_prc']
    plt.hist(arr_cmb, bins=40, histtype='step', range=[0, 1], label='CMB')
    plt.hist(arr_prc, bins=40, histtype='step', range=[0, 1], label='PRC')

    plt.legend()
    plt.savefig(f'{Data.out_dir}/{test}_mva.png')
    plt.close()
# ------------------------------------------------
def test_simple():
    '''
    Simplest test of getter class
    '''
    RDFGetter.samples_dir = '/publicfs/ucas/user/campoverde/Data/RX_run3/v4/NO_q2_bdt_mass_Q2_central_VR_v1'

    gtr = RDFGetter(sample='DATA_24_MagUp_24c2', trigger='Hlt2RD_BuToKpMuMu_MVA')
    rdf = gtr.get_rdf()

    _check_branches(rdf)
    _plot_mva_mass(rdf, 'simple')
    _plot_mva(rdf, 'simple')
