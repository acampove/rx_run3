'''
Module containing DataFitter class
'''
from omegaconf                import DictConfig, OmegaConf

from dmu.stats.zfit           import zfit
from dmu.logging.log_store    import LogStore
from rx_common.types          import Trigger
from rx_selection             import selection  as sel

from zfit.loss                import ExtendedUnbinnedNLL
from zfit.interface           import ZfitSpace  as zobs
from fitter.data_preprocessor import DataPreprocessor
from fitter.data_model        import DataModel

log=LogStore.add_logger('fitter:LikelihoodFactory')
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
        sample  : str,
        q2bin   : str,
        cfg     : DictConfig,
        name    : str|None = None):
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
    def _get_base_path(self) -> str:
        '''
        Returns directory where outputs will go
        '''
        sample = self._sample.replace('*', 'p')
        if self._name is not None:
            sample = f'{self._cfg.output_directory}/{sample}/{self._name}/{self._trigger}_{self._q2bin}'
        else:
            sample = f'{self._cfg.output_directory}/{sample}/{self._trigger}_{self._q2bin}'

        return sample
    # ------------------------
    def _update_mc_trigger(self) -> Trigger:
        '''
        Returns
        -------------
        Trigger that will be used for simulation components

        If trigger does not end with ext return _trigger
        If trigger ends with ext, switch to noPID trigger. Because we are working with PID control region with simulation.
        '''
        if self._trigger not in [Trigger.rk_ee_ext, Trigger.rkst_ee_ext]:
            log.debug(f'Found non-Extended trigger {self._trigger}, using default project')
            return self._trigger 

        value   = self._trigger.replace('_ext', '_noPID')
        trigger = Trigger(value)

        log.info(f'Found ext trigger, overriding with: {trigger}')

        return trigger
    # ------------------------
    def run(self) -> ExtendedUnbinnedNLL:
        '''
        Creates negative log-likelihood

        Returns
        ------------
        Zfit likelihood
        '''
        log.info('Getting data')
        dpr  = DataPreprocessor(
            obs    = self._obs,
            q2bin  = self._q2bin,
            sample = self._sample,
            trigger= self._trigger,
            out_dir= self._base_path,
            wgt_cfg= None) # Do not need weights for data
        data = dpr.get_data()

        trigger = self._update_mc_trigger()

        log.info('Getting full data model using fits to simulation')
        log.debug(f'{"Trigger":<20}{trigger}')
        log.debug(f'{"q2bin  ":<20}{self._q2bin}')
        mod  = DataModel(
            name   = self._name,
            cfg    = self._cfg,
            obs    = self._obs,
            q2bin  = self._q2bin,
            trigger= trigger)
        model= mod.get_model()

        log.info(50 * '-')
        log.info(f'Making likelihood for: {self._sample}/{self._trigger}/{self._q2bin}')
        log.info(50 * '-')

        nll = zfit.loss.ExtendedUnbinnedNLL(model=model, data=data)

        return nll
    # ----------------------
    def get_config(self) -> DictConfig:
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

        cfg = {
            'selection' :
            {
                'default' : cuts_def,
                'fit'     : cuts_fit}
        }

        return OmegaConf.create(obj=cfg)
# ------------------------
