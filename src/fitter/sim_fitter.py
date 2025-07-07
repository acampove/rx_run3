'''
Module with SimFitter class
'''

from omegaconf                import DictConfig
from zfit.core.interfaces     import ZfitPDF      as zpdf
from zfit.core.interfaces     import ZfitSpace    as zobs
from dmu.stats.model_factory  import ModelFactory
from fitter.base_fitter       import BaseFitter
from fitter.sim_model         import SimModel
from fitter.data_preprocessor import DataPreprocessor

# ------------------------
class SimFitter(BaseFitter):
    '''
    Fitter for simulation samples
    '''
    # ------------------------
    def __init__(
        self,
        name : str,
        cfg  : DictConfig,
        obs  : zobs):
        '''
        Parameters
        --------------------
        obs  : Observable
        name : Nickname of component, e.g. combinatorial
        cfg  : Object storing configuration for fit
        '''
        self._cfg = cfg
        self._obs = obs
        self._name= name
    # ------------------------
    def _get_pdf(self) -> zpdf:
        l_model = self._cfg.models
        mod     = ModelFactory(
            preffix = self._name,
            obs     = self._obs,
            l_pdf   = l_model,
            l_shared= self._cfg.shared,
            l_float = self._cfg.float ,
            d_rep   = self._cfg.reparametrize,
            d_fix   = self._cfg.fix)

        pdf = mod.get_pdf()

        return pdf
    # ------------------------
    def get_model(self) -> zpdf:
        '''
        Returns
        ------------
        zfit PDF, not extended yet
        '''
        pdf = self._get_pdf()
        if 'simulation' not in self._cfg:
            return pdf

        return pdf
# ------------------------
