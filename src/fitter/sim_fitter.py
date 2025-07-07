'''
Module with SimFitter class
'''

from omegaconf               import DictConfig
from zfit.core.interfaces    import ZfitPDF    as zpdf
from zfit.core.interfaces    import ZfitSpace  as zobs
from dmu.stats.model_factory import ModelFactory
from fitter.base_fitter      import BaseFitter
from fitter.sim_model        import SimModel

# ------------------------
class SimFitter(BaseFitter):
    '''
    Fitter for simulation samples
    '''
    # ------------------------
    def __init__(
        self,
        cfg : DictConfig,
        obs : zobs):
        '''
        Parameters
        --------------------
        obs : Observable
        cfg : Object storing configuration for fit
        '''
        self._cfg = cfg
        self._obs = obs
    # ------------------------
    def get_model(self) -> zpdf:
        '''
        Returns
        ------------
        zfit PDF, not extended yet
        '''
# ------------------------
