'''
Module testing KinematicsVarsAdder class
'''
import os
import pytest

from ROOT import RDataFrame

from dmu.logging.log_store   import LogStore
from post_ap.part_vars_adder import ParticleVarsAdder

log = LogStore.add_logger('post_ap:test_part_vars_adder')

# ---------------------------------------------
class Data:
    '''
    Class with shared attributes
    '''
    l_data_type = ['dt', 'mc']
# ---------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('post_ap:part_vars_adder', 10)
# ---------------------------------------------
def _get_rdf(data_type : str) -> RDataFrame:
    cernbox   = os.environ['CERNBOX']
    file_path = f'{cernbox}/Run3/analysis_productions/for_local_tests/{data_type}_turbo.root'
    tree_path =  'Hlt2RD_BuToKpMuMu_MVA/DecayTree'

    if not os.path.isfile(file_path):
        raise FileNotFoundError(f'Cannot find: {file_path}')

    rdf = RDataFrame(tree_path, file_path)

    return rdf
# ---------------------------------------------
@pytest.mark.parametrize('data_type', Data.l_data_type)
def test_simple(data_type : str):
    '''
    Simplest test of adding variables
    '''
    d_expr = {
    'P'  : 'TMath::Sqrt( TMath::Sq(PARTICLE_PX) + TMath::Sq(PARTICLE_PY) + TMath::Sq(PARTICLE_PZ) )',
    'PT' : 'TMath::Sqrt( TMath::Sq(PARTICLE_PX) + TMath::Sq(PARTICLE_PY) )',
    }

    rdf = _get_rdf(data_type=data_type)
    obj = ParticleVarsAdder(rdf, variables = d_expr)
    rdf = obj.get_rdf()

    l_name    = obj.names
    file_path = '/tmp/par_var_add.root'

    log.info(f'Saving to: {file_path}')
    rdf.Snapshot('tree', file_path, l_name)
