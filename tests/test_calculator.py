'''
Module meant to test the AcceptanceCalculator class
'''

from ROOT import RDataFrame

from rx_efficiencies.acceptance_calculator import AcceptanceCalculator

#--------------------------
def _get_rdf():
    file_path = '/home/acampove/Test/acceptance/bsphiee_tree.root'

    rdf = RDataFrame('DecayTree', file_path)

    return rdf
#--------------------------
def test_simple():
    '''
    Simplest test of geometric acceptance calculation
    '''
    rdf=_get_rdf()
    obj=AcceptanceCalculator(rdf=rdf)
    obj.plot_dir     = 'tests/calculator/simple'
    acc_phy, acc_lhc = obj.get_acceptances()
#--------------------------
