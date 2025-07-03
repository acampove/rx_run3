'''
Module meant to test MisIDDataset class
'''

import pytest

from dmu.logging.log_store  import LogStore
from rx_misid.misid_dataset import MisIDDataset

# -----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str):
    '''
    Simplest test
    '''
    dst = MisIDDataset(q2bin=q2bin)
    d_df= dst.get_data()
# -----------------------------------------------
