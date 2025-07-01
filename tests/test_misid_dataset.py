'''
Module meant to test MisIDDataset class
'''

from rx_data.rdf_getter     import RDFGetter
from rx_misid.misid_dataset import MisIDDataset

# -----------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    dst = MisIDDataset(q2bin='central')
    d_df= dst.get_data()
# -----------------------------------------------
