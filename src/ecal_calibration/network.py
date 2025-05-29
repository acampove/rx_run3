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
    def __init__(self):
        super().__init__()
        self.model = nn.Sequential(
            nn.Linear(6, 6),
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
