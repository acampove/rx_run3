'''
Module meant to test the AcceptanceCalculator class
'''

import os

import pytest
from ROOT import RDataFrame
from rx_efficiencies.acceptance_calculator import AcceptanceCalculator

#--------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/rx_efficiencies/acceptance_calculator'
    rsm_dir = os.environ['RAPIDSIM_NTUPLES']
#--------------------------
def _get_rdf(sample : str) -> RDataFrame:
    file_path = f'{Data.rsm_dir}/{sample}/{sample}_tree.root'
    rdf       = RDataFrame('DecayTree', file_path)

    return rdf
#--------------------------
@pytest.mark.parametrize('sample', ['bdkskpiee'])
def test_sample(sample : str):
    '''
    Simplest test of geometric acceptance calculation
    '''
    rdf=_get_rdf(sample=sample)
    obj=AcceptanceCalculator(rdf=rdf)
    obj.plot_dir     = f'{Data.out_dir}/{sample}'
    acc_phy, acc_lhc = obj.get_acceptances()
#--------------------------
