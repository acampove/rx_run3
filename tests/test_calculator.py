from acceptance_calculator import calculator

import ROOT

#--------------------------
def get_rdf():
    #file_path = '/home/acampove/Test/acceptance/B2Kee_tree.root'
    file_path = '/home/acampove/Test/acceptance/bsphiee_tree.root'
    #file_path = '/home/acampove/Test/acceptance/bdkskpiee_tree.root'

    rdf = ROOT.RDataFrame('DecayTree', file_path)

    return rdf
#--------------------------
def test_simple():
    rdf=get_rdf()
    obj=calculator(rdf)
    obj.plot_dir     = 'tests/calculator/simple'
    acc_phy, acc_lhc = obj.get_acceptances()
#--------------------------
def main():
    test_simple()
#--------------------------
if __name__ == '__main__':
    main()

