'''
Module used to hold:

Network: Class meant to represent a neural network, used for actual prediction
ConstantModel: Used for debugging purposes, outputs the same value for any input
'''

import torch
from torch import nn
from torch import Tensor

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ecal_calibration:network')
# --------------------------------------
class Network(nn.Module):
    '''
    Class wrapping pytorch (abstract?) newtwork
    '''
    # ------------------------------
    def __init__(self, nfeatures : int, model : str):
        '''
        nfeatures (int): Number of features, needed to build first layer
        model          : Name of model, e.g. v1
        '''
        self._nfeatures = nfeatures

        super().__init__()

        fun = getattr(self, f'_model_{model}', None)
        if fun is None:
            raise ValueError(f'No model {model} found')

        self.model      = fun()
    # ------------------------------
    def _model_v1(self) -> nn.Sequential:
        log.info('Using model v1')

        model = nn.Sequential(
            nn.Linear(self._nfeatures, 6),
            nn.ReLU(),
            nn.Linear(6,               1)
        )

        return model
    # ------------------------------
    def _model_v2(self) -> nn.Sequential:
        log.info('Using model v2')

        model = nn.Sequential(
            nn.Linear(self._nfeatures, 6),
            nn.ReLU(),
            nn.Linear(6              , 6),
            nn.ReLU(),
            nn.Linear(6              , 1)
        )

        return model
    # ------------------------------
    def _model_v3(self) -> nn.Sequential:
        log.info('Using model v3')

        model = self.model = nn.Sequential(
            nn.Linear(self._nfeatures, 10),
            nn.ReLU(),
            nn.Linear(10             , 10),
            nn.ReLU(),
            nn.Linear(10             ,  1)
        )

        return model
    # ------------------------------
    def _model_v4(self) -> nn.Sequential:
        log.info('Using model v4')

        model = self.model = nn.Sequential(
            nn.Linear(self._nfeatures,  6),
            nn.ReLU(),
            nn.Linear(6              ,  6),
            nn.ReLU(),
            nn.Linear(6              ,  6),
            nn.ReLU(),
            nn.Linear(6              ,  1)
        )

        return model
    # ------------------------------
    def _model_v5(self) -> nn.Sequential:
        log.info('Using model v5')

        model = self.model = nn.Sequential(
            nn.Linear(self._nfeatures, 15),
            nn.ReLU(),
            nn.Linear(15             , 15),
            nn.ReLU(),
            nn.Linear(15             ,  1)
        )

        return model
    # ------------------------------
    def _model_v6(self) -> nn.Sequential:
        log.info('Using model v6')

        model = self.model = nn.Sequential(
            nn.Linear(self._nfeatures, 15),
            nn.ReLU(),
            nn.Linear(15             , 15),
            nn.ReLU(),
            nn.Linear(15             , 15),
            nn.ReLU(),
            nn.Linear(15             ,  1)
        )

        return model
    # ------------------------------
    def _model_v7(self) -> nn.Sequential:
        log.info('Using model v7')

        model = self.model = nn.Sequential(
            nn.Linear(self._nfeatures, 20),
            nn.ReLU(),
            nn.Linear(20             , 20),
            nn.ReLU(),
            nn.Linear(20             , 20),
            nn.ReLU(),
            nn.Linear(20             ,  1)
        )

        return model
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
