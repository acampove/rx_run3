'''
Module containing DataModel class
'''

from dmu.stats.zfit         import zfit
from dmu.generic            import utilities  as gut
from dmu.logging.log_store  import LogStore

from omegaconf              import DictConfig
from zfit.core.interfaces   import ZfitPDF    as zpdf
from zfit.core.interfaces   import ZfitSpace  as zobs

from fitter.base_model      import BaseModel
from fitter.sim_fitter      import SimFitter

log = LogStore.add_logger('fitter:data_model')
# ------------------------
class DataModel(BaseModel):
    '''
    Model for fitting data samples
    '''
    # ------------------------
    def __init__(
            self,
            cfg : DictConfig,
            obs : zobs):
        '''
        Parameters
        ------------------
        cfg : Configuration object
        obs : zfit observable
        '''
        self._cfg = cfg
        self._obs = obs
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

        nevt = zfit.param.Parameter(f'n{name}', 100, 0, 1000_000)
        kdes = zfit.pdf.KDE1DimFFT, zfit.pdf.KDE1DimExact, zfit.pdf.KDE1DimISJ
        if isinstance(pdf, kdes):
            pdf.set_yield(nevt)
            return pdf

        pdf = pdf.create_extended(nevt, name=name)

        return pdf
    # ------------------------
    def get_model(self) -> zpdf:
        '''
        Returns fitting model for data fit
        '''
        l_pdf = []
        npdf  = len(self._cfg.model)
        if npdf == 0:
            log.info(self._cfg.model)
            raise ValueError('Found zero components in model')

        log.debug(f'Found {npdf} componets')
        for component, cfg_path in self._cfg.model.items():
            cfg = gut.load_conf(package='fitter_data', fpath=cfg_path)
            ftr = SimFitter(
                name=component,
                cfg =cfg,
                obs =self._obs)
            pdf = ftr.get_model()
            pdf = self._extend(pdf=pdf, name=component)

            l_pdf.append(pdf)

        pdf = zfit.pdf.SumPDF(l_pdf)

        return pdf
# ------------------------
