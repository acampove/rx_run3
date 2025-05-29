'''
Module used to hold Network class
'''
from torch import nn
from torch import Tensor

# --------------------------------------
class Network(nn.Module):
    '''
    Class wrapping pytorch (abstract?) newtwork
    '''
    # ------------------------------
    def __init__(self, nfeatures : int):
        '''
        nfeatures (int): Number of features, needed to build first layer
        '''
        super().__init__()
        self.model = nn.Sequential(
            nn.Linear(nfeatures, 6),
            nn.ReLU(),
            nn.Linear(6, 1)
        )
    # ------------------------------
    def forward(self, x : Tensor) -> Tensor:
        '''
        Evaluates the features through the model
        '''
        return self.model(x)
# --------------------------------------
