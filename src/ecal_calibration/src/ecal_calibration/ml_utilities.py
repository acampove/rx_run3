'''
This module contains utility functions that need Torch
'''

from torch  import Tensor

# ------------------------------------
def normalize_tensor(x : Tensor) -> Tensor:
    '''
    Makes sure the mean of the distribution is zero and the standard deviation is 1
    '''
    mean = x.mean()
    std  = x.std()
    x    = (x - mean) / std

    return x

