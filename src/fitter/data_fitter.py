'''
Module containing DataFitter class
'''

from omegaconf          import DictConfig, OmegaConf
from fitter.base_fitter import BaseFitter
from fitter.data_model  import DataModel

# ------------------------
class DataFitter(BaseFitter):
    '''
    Fitter for data
    '''
    # ------------------------
    def __init__(self, cfg : DictConfig):
        '''
        cfg : configuration for the fit as a DictConfig object
        '''
        self._cfg = cfg
    # ------------------------
    def run(self) -> DictConfig:
        '''
        Runs fit

        Returns
        ------------
        DictConfig object with fitting results
        '''
        res = OmegaConf.create({
            'mu' : {'value' : 1.0,
                    'error' : 0.1}})

        return res
# ------------------------
