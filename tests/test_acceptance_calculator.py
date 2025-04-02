'''
Module meant to test the AcceptanceCalculator class
'''

import os

import pytest
import mplhep
import matplotlib.pyplot as plt
from ROOT                                  import RDataFrame
from dmu.logging.log_store                 import LogStore
from rx_efficiencies.acceptance_calculator import AcceptanceCalculator

log=LogStore.add_logger('rx_efficiencies:test_acceptance_calculator')
#--------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/rx_efficiencies/acceptance_calculator'
    rsm_dir = os.environ['RAPIDSIM_NTUPLES']

    plt.style.use(mplhep.style.LHCb2)
#--------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_efficiencies:test_acceptance_calculator', 10)
#--------------------------
def _get_rdf(sample : str, energy : str) -> RDataFrame:
    file_path = f'{Data.rsm_dir}/{sample}/{energy}/{sample}_tree.root'
    rdf       = RDataFrame('DecayTree', file_path)

    return rdf
#--------------------------
@pytest.mark.parametrize('sample', ['bdkskpiee', 'bpkskpiee', 'bsphiee'])
@pytest.mark.parametrize('energy', ['8TeV', '13TeV', '14TeV'])
def test_sample(sample : str, energy : str):
    '''
    Simplest test of geometric acceptance calculation for different samples
    '''
    rdf=_get_rdf(sample=sample, energy=energy)
    obj=AcceptanceCalculator(rdf=rdf)
    obj.plot_dir     = f'{Data.out_dir}/{sample}/{energy}'
    acc_phy, acc_lhc = obj.get_acceptances()

    log.info(f'{sample:<20}{acc_phy:10.3f}{acc_lhc:10.3f}')

    assert acc_phy >= acc_lhc
#--------------------------
