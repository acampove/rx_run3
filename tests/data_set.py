import logging

from q2_syst.data_set import data_set 

#-----------------------------------
def test_simple():
    obj = data_set(is_mc=True, trigger='ETOS', dset='r1')
    obj.log.setLevel(logging.DEBUG)
    #obj.nentries = 1000
    obj.plt_dir  = 'tests/data_set/simple'
    rdf = obj.get_rdf()
#-----------------------------------
if __name__ == '__main__':
    test_simple()

