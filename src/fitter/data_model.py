'''
Module containing DataModel class
'''

from dmu.stats.zfit         import zfit
from dmu.logging.log_store  import LogStore
from dmu.stats.parameters   import ParameterLibrary as PL
from omegaconf              import DictConfig
from zfit.pdf               import BasePDF       as zpdf
from zfit.interface         import ZfitSpace     as zobs
from fitter.sim_fitter      import SimFitter

log = LogStore.add_logger('fitter:data_model')
# ------------------------
class DataModel:
    '''
    Model for fitting data samples
    '''
    # ------------------------
    def __init__(
        self,
        cfg     : DictConfig,
        obs     : zobs,
        trigger : str,
        project : str,
        q2bin   : str,
        name    : str|None=None):
        '''
        Parameters
        ------------------
        cfg    : Configuration object
        trigger: Hlt2RD...
        project: E.g. rx
        q2bin  : E.g. central
        obs    : zfit observable
        name   : Optional, identifier for this model, 
                 if passed, used also as suffix of yields, e.g. yld_combinatorial_{name}
        '''
        self._cfg    = cfg
        self._obs    = obs
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._name   = name
    # ------------------------
    def _extend(self, pdf : zpdf, name : str) -> zpdf:
        '''
        Parameters
        -------------------
        name: Name of component
        pdf : zfit pdf

        Returns
        -------------------
        PDF with yield
        '''
        yield_name = f'yld_{name}' if self._name is None else f'yld_{name}_{self._name}'

        nevt = PL.get_yield(name=yield_name)
        kdes = zfit.pdf.KDE1DimFFT, zfit.pdf.KDE1DimExact, zfit.pdf.KDE1DimISJ
        if isinstance(pdf, kdes):
            pdf.set_yield(nevt)
            return pdf

        pdf = pdf.create_extended(nevt, name=name)

        return pdf
    # ------------------------
    def get_model(self) -> zpdf:
        '''
        Returns 
        ----------------
        Fitting model for data fit
        '''
        l_pdf = []
        npdf  = len(self._cfg.model)
        if npdf == 0:
            log.info(self._cfg.model)
            raise ValueError('Found zero components in model')

        log.debug(f'Found {npdf} components')
        for component, cfg in self._cfg.model.components.items():
            ftr = SimFitter(
                name     = self._name,
                component= component,
                trigger  = cfg.get('trigger', self._trigger),
                project  = cfg.get('project', self._project),
                q2bin    = self._q2bin,
                cfg      = cfg,
                obs      = self._obs)
            pdf = ftr.get_model()

            if pdf is None:
                log.warning(f'Skipping component: {component}')
                continue

            pdf = self._extend(pdf=pdf, name=component)
            l_pdf.append(pdf)

        pdf = zfit.pdf.SumPDF(l_pdf)

        return pdf
# ------------------------
