'''
Module containing tests for HOPVarAdder
'''
import os

import yaml
import pytest
import matplotlib.pyplot as plt
from ROOT                   import RDataFrame
from rx_data.hop_calculator import HOPCalculator

# ----------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir = '/tmp/rx_data/tests/hop_calculator'
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

    return rdf
# ----------------------------
def _plot_variables(rdf : RDataFrame, rdf_hop : RDataFrame, name : str) -> None:
    out_dir = f'{Data.out_dir}/{name}'
    os.makedirs(out_dir, exist_ok=True)

    d_data = rdf_hop.AsNumpy(['alpha', 'mass'])
    arr_ms = rdf.AsNumpy(['B_M'])['B_M']

    plt.hist(d_data['alpha'], bins=40, range=[0,5])
    plt.savefig(f'{out_dir}/alpha.png')
    plt.close()

    plt.hist(d_data['mass'], bins=40, label='HOP')
    plt.hist(        arr_ms, bins=40, label='Original')
    plt.legend()
    plt.savefig(f'{out_dir}/mass.png')
    plt.close()
# ----------------------------
@pytest.mark.parametrize('sample', ['Bd_Kstee_eq_btosllball05_DPC', 'Bu_Kee_eq_btosllball05_DPC'])
def test_simple(sample : str):
    '''
    Simplest test
    '''
    rdf = _get_rdf(sample = sample, trigger='Hlt2RD_BuToKpEE_MVA')

    obj     = HOPCalculator(rdf=rdf)
    rdf_hop = obj.get_rdf()

    _plot_variables(rdf=rdf, rdf_hop=rdf_hop, name=f'simple_{sample}')
# ----------------------------

# Bd_Kstmumu_eq_btosllball05_DPC                                                                                                                                                                                                        
# Bu_Kmumu_eq_btosllball05_DPC                                                                                                                                                                                                          
# Bd_Kstee_eq_btosllball05_DPC                                                                                                                                                                                                          
# Bu_Kee_eq_btosllball05_DPC                                                                                                                                                                                                            
# Bd_JpsiX_ee_eq_JpsiInAcc                                                                                                                                                                                                              
# Bu_JpsiX_ee_eq_JpsiInAcc                                                                                                                                                                                                              
# Bs_JpsiX_ee_eq_JpsiInAcc                                                                                                                                                                                                              
# Lb_JpsiX_ee_eq_JpsiInAcc     
