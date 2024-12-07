'''
Module testing KinematicsVarsAdder class
'''
import os

from ROOT import RDataFrame

from dmu.logging.log_store   import LogStore
from post_ap.kine_vars_adder import KinematicsVarsAdder

log = LogStore.add_logger('post_ap:test_kine_vars_adder')
# ---------------------------------------------
def _get_rdf() -> RDataFrame:
    cernbox   = os.environ['CERNBOX']
    file_path = f'{cernbox}/Run3/analysis_productions/for_local_tests/data.root'
    tree_path =  'Hlt2RD_BuToKpEE_MVA/DecayTree'

    if not os.path.isfile(file_path):
        raise FileNotFoundError(f'Cannot find: {file_path}')

    rdf = RDataFrame(tree_path, file_path)

    return rdf
# ---------------------------------------------
def test_simple():
    '''
    Simplest test of adding variables
    '''
    rdf = _get_rdf()
    obj = KinematicsVarsAdder(rdf, variables = ['PT', 'P'])
    rdf = obj.get_rdf()
