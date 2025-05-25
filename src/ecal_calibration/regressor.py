'''
Module containing the Regressor class
'''
import os

import torch
from torch import nn
from torch import optim
from torch import Tensor

import pandas as pnd
from dmu.logging.log_store    import LogStore
from ecal_calibration.network import Network

log=LogStore.add_logger('ecal_calibration:regressor')
# ---------------------------------------------
class Regressor:
    '''
    Class used to train a regressor to _learn_ energy
    corrections
    '''
    # ---------------------------------------------
    def __init__(self, df : pnd.DataFrame, cfg : dict):
        '''
        Parameters
        -------------------
        df  : Pandas dataframe storing the target and the features
        cfg : Dictionary holding configuration
        '''
    # ---------------------------------------------
    def train(self) -> None:
        '''
        Will train the regressor
        '''
    # ---------------------------------------------
    def test(self) -> None:
        '''
        Will test performance of regressor
        '''
# ---------------------------------------------
