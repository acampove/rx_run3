'''
Module containing DataModel class
'''

from dmu.stats.zfit         import zfit
from dmu.generic            import utilities  as gut
from dmu.logging.log_store  import LogStore

from omegaconf              import DictConfig
from zfit                   import ComposedParameter
from zfit.interface         import ZfitPDF       as zpdf
from zfit.interface         import ZfitParameter as zpar
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
        name   : Optional, identifier for this model
        '''
        self._cfg    = cfg
        self._obs    = obs
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._name   = name
        self._nsig   = zfit.param.Parameter('nsignal', 100, 0, 1000_000)

        self._prc_pref = 'pscale' # This is the partially reconstructed scale preffix.
                                  # The name here mirrors what is in ConstraintReader.
    # ------------------------
    def _get_yield(self, name : str) -> zpar|ComposedParameter:
        '''
        Parameters
        --------------
        name: Sample name, e.g. Bu_Kee_eq_btosllball05_DPC.
              If component does not have MC associated, component name e.g. combinatorial

        Returns
        --------------
        zfit parameter used for extending it
        '''
        if name == 'signal':
            return self._nsig

        if name not in self._cfg.model.constraints.yields:
            log.debug(f'Yield for component {name} will be non-composed')
            return zfit.param.Parameter(f'n{name}', 100, 0, 1000_000)

        log.info(f'Yield for component {name} will be composed')
        # This scale should normally be below 1
        # It is nbackground / nsig
        # The parameter HAS TO start with pscale such that it is picked
        # by ConstraintReader
        scale= zfit.Parameter(f'{self._prc_pref}{name}', 0, 0, 10)
        nevt = zfit.ComposedParameter(f'n{name}', lambda x : x['nsig'] * x['scale'], params={'nsig' : self._nsig, 'scale' : scale})

        return nevt
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
        nevt = self._get_yield(name=name)

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

        log.debug(f'Found {npdf} components')
        for component, cfg_path in self._cfg.model.components.items():
            cfg = gut.load_conf(package='fitter_data', fpath=cfg_path)
            ftr = SimFitter(
                name     = self._name,
                component= component,
                trigger  = self._trigger,
                project  = self._project,
                q2bin    = self._q2bin,
                cfg      = cfg,
                obs      = self._obs)
            pdf = ftr.get_model()

            if pdf is None:
                log.warning(f'Skipping component: {component}')
                continue

            if component == 'signal':
                sample = component
            else:
                sample = cfg.get(key='sample', default_value=component)

            pdf    = self._extend(pdf=pdf, name=sample)
            l_pdf.append(pdf)

        pdf = zfit.pdf.SumPDF(l_pdf)

        return pdf
# ------------------------
