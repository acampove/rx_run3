from q2_syst.data_set import dset


#-----------------------------------
def test_simple():
    obj = dset(is_mc=True, trigger='ETOS', dset='r1')
    rdf = obj.get_rdf()
#-----------------------------------
if __name__ == '__main__':
    test_simple()

