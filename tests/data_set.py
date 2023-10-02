import logging

from q2_syst.data_set import data_set 

#-----------------------------------
def test_mc():
    obj = data_set(is_mc=True, trigger='ETOS', dset='r1')
    obj.log.setLevel(logging.DEBUG)
    #obj.nentries = 1000
    obj.plt_dir  = 'tests/data_set/mc'
    rdf = obj.get_rdf()
#-----------------------------------
def test_dt():
    obj = data_set(is_mc=False, trigger='ETOS', dset='r1')
    obj.log.setLevel(logging.DEBUG)
    #obj.nentries = 1000
    obj.plt_dir  = 'tests/data_set/dt'
    rdf = obj.get_rdf()
#-----------------------------------
if __name__ == '__main__':
    test_dt()
    #test_mc()

