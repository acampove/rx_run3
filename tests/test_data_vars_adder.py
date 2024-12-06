'''
Module containing tests for DataVarsAdder class
'''
import os

from ROOT import RDataFrame

from post_ap.data_vars_adder import DataVarsAdder

# ---------------------------------------------
def _get_rdf():
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
    Simplest test for adding data variables
    '''

    rdf = _get_rdf()

    obj = DataVarsAdder(rdf)
    rdf = obj.get_rdf()
