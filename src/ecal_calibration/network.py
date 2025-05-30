'''
Module used to hold:

Network: Class meant to represent a neural network, used for actual prediction
ConstantModel: Used for debugging purposes, outputs the same value for any input
'''

import torch
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
class ConstantModel(nn.Module):
    '''
    Model used for debugging purposes, it outputs a given constant
    '''
    # ------------------------------
    def __init__(self, target : float):
        super().__init__()
        self.value = nn.Parameter(torch.tensor(target))
    # ------------------------------
    def forward(self, x: Tensor) -> Tensor:
        '''
        Returns required target, no training needed
        '''
        return self.value.expand(x.shape[0], 1)
# --------------------------------------
