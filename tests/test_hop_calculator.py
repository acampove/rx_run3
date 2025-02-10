'''
Module containing tests for HOPVarAdder
'''

import yaml
from ROOT import RDataFrame

from post_ap.hop_vars_adder import HOPVarAdder as HVA

# ----------------------------
def _get_samples() -> dict:
    samples_list='/publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml'
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
def test_simple():
    '''
    Simplest test
    '''
    rdf = _get_rdf(sample = 'Bd_Kstee_eq_btosllball05_DPC', trigger='Hlt2RD_BuToKpEE_MVA')

    obj = HVA(rdf=rdf)
    rdf = obj.get_rdf()
# ----------------------------

# Bd_Kstmumu_eq_btosllball05_DPC                                                                                                                                                                                                        
# Bu_Kmumu_eq_btosllball05_DPC                                                                                                                                                                                                          
# Bd_Kstee_eq_btosllball05_DPC                                                                                                                                                                                                          
# Bu_Kee_eq_btosllball05_DPC                                                                                                                                                                                                            
# Bd_JpsiX_ee_eq_JpsiInAcc                                                                                                                                                                                                              
# Bu_JpsiX_ee_eq_JpsiInAcc                                                                                                                                                                                                              
# Bs_JpsiX_ee_eq_JpsiInAcc                                                                                                                                                                                                              
# Lb_JpsiX_ee_eq_JpsiInAcc     
