'''
Module meant to test MisIDDataset class
'''

from rx_misid.misid_dataset import MisIDDataset

def test_simple():
    '''
    Simplest test
    '''
    dst = MisIDDataset(version='v1', q2bin='central')
    d_df= dst.get_data()
