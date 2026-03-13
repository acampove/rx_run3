'''
Module containing DataFitter class
'''
from pathlib            import Path

from dmu                import LogStore
from dmu.stats          import zfit
from rx_common          import Qsq, Component
from rx_selection       import selection  as sel

from zfit.loss          import ExtendedUnbinnedNLL
from zfit               import Space  as zobs

from .configs           import FitModelConf
from .data_preprocessor import DataPreprocessor
from .data_model        import DataModel

log=LogStore.add_logger('fitter:likelihood_factory')
# ------------------------
class LikelihoodFactory:
    '''
    Builder of likelihoods for real data (not MC) given a:

    - Data sample
    - Configuration describing the models to use
    '''
    # ------------------------
    def __init__(
        self,
        obs     : zobs,
        sample  : Component,
        q2bin   : Qsq,
        cfg     : FitModelConf,
        name    : str | None = None):
        '''
        name   : Identifier for fit, e.g. block. This is optional
        cfg    : configuration for the fit as a DictConfig object
        sample : Identifies sample e.g. DATA_24_MagUp...
        project: E.g. rx
        q2bin  : E.g. central
        cfg    : Configuration for the fit to data
        '''
        self._obs       = obs
        self._sample    = sample
        self._trigger   = cfg.trigger
        self._q2bin     = q2bin
        self._cfg       = cfg
        self._name      = name
        self._base_path = self._get_base_path()
    # ------------------------
    def _get_base_path(self) -> Path:
        '''
        Returns directory where outputs will go
        '''
        if self._name is None:
            return self._cfg.output_directory / f'{self._trigger}_{self._q2bin}'

        return self._cfg.output_directory / f'{self._name}/{self._trigger}_{self._q2bin}'
    # ------------------------
    def run(self) -> ExtendedUnbinnedNLL:
        '''
        Creates negative log-likelihood

        Returns
        ------------
        Zfit likelihood
        '''
        log.info('Getting full data model using fits to simulation')
        log.debug(f'{"Trigger":<20}{self._trigger}')
        log.debug(f'{"q2bin  ":<20}{self._q2bin}')
        mod  = DataModel(
            name   = self._name,
            cfg    = self._cfg,
            obs    = self._obs,
            q2bin  = self._q2bin,
            trigger= self._trigger)
        model= mod.get_model()

        log.info('Getting data')
        dpr  = DataPreprocessor(
            obs    = self._obs,
            q2bin  = self._q2bin,
            sample = self._sample,
            trigger= self._trigger,
            out_dir= self._base_path,
            wgt_cfg= None) # Do not need weights for data
        data = dpr.get_data()

        log.info(50 * '-')
        log.info(f'Making likelihood for: {self._sample}/{self._trigger}/{self._q2bin}')
        log.info(50 * '-')

        nll = zfit.loss.ExtendedUnbinnedNLL(model=model, data=data)

        return nll
    # ----------------------
    def get_config(self) -> dict[str,dict[str,str]]:
        '''
        Returns
        -------------
        OmegaConf dictionary storing the configuration, which includes:

        - Selection
        '''
        log.debug('Retrieving NLL configuration')

        cuts_fit = sel.selection(
            process=self._sample,
            trigger=self._trigger,
            q2bin  =self._q2bin)

        with sel.custom_selection(d_sel={}, force_override=True):
            cuts_def = sel.selection(
                process=self._sample,
                trigger=self._trigger,
                q2bin  =self._q2bin)

        return {
            'default' : cuts_def,
            'fit'     : cuts_fit}
# ------------------------
