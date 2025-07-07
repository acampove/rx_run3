'''
Module with SimFitter class
'''

from typing import cast

from omegaconf                import DictConfig, OmegaConf
from zfit.core.interfaces     import ZfitPDF      as zpdf
from zfit.core.interfaces     import ZfitSpace    as zobs
from dmu.workflow.cache       import Cache
from dmu.stats.model_factory  import ModelFactory
from dmu.logging.log_store    import LogStore
from fitter.base_fitter       import BaseFitter
from fitter.data_preprocessor import DataPreprocessor

log=LogStore.add_logger('fitter:sim_fitter')
# ------------------------
class SimFitter(BaseFitter, Cache):
    '''
    Fitter for simulation samples
    '''
    # ------------------------
    def __init__(
        self,
        name    : str,
        trigger : str,
        project : str,
        q2bin   : str,
        cfg     : DictConfig,
        obs     : zobs):
        '''
        Parameters
        --------------------
        obs    : Observable
        name   : Nickname of component, e.g. combinatorial
        trigger: Hlt2RD...
        project: E.g. rx
        q2bin  : E.g. central
        cfg    : Object storing configuration for fit
        '''
        self._name   = name
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._cfg    = cfg
        self._obs    = obs
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
    def _fix_tails(self, pdf : zpdf, pars : DictConfig) -> zpdf:
        '''
        Parameters
        --------------
        pdf : PDF after fit
        pars:
        '''
        s_par = pdf.get_params()
        for par in s_par:
            # Model builder adds _flt to name
            # of parameters meant to float
            if par.name.endswith('_flt'):
                continue

            if par.name in pars:
                par.set_value(pars.value)
                log.debug(f'{par.name:<20}{"--->"}{pars.value:<20.3f}')
                par.floating = False

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

        prp = DataPreprocessor(obs=self._obs, **self._cfg.simulation)
        data= prp.get_data()

        self._fit(data=data, pdf=pdf)

        return pdf
# ------------------------
