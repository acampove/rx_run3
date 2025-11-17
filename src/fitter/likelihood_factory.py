'''
Module containing DataFitter class
'''
from omegaconf                import DictConfig, OmegaConf

from dmu.stats.zfit           import zfit
from dmu.logging.log_store    import LogStore
from rx_common.types          import Trigger
from rx_common                import info
from rx_selection             import selection  as sel
from rx_data.spec_maker       import SpecMaker

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
        trigger : Trigger,
        q2bin   : str,
        cfg     : DictConfig,
        name    : str|None = None):
        '''
        name   : Identifier for fit, e.g. block. This is optional
        cfg    : configuration for the fit as a DictConfig object
        sample : Identifies sample e.g. DATA_24_MagUp...
        trigger: Hlt2RD...
        project: E.g. rx
        q2bin  : E.g. central
        cfg    : Configuration for the fit to data
        '''
        self._obs       = obs
        self._sample    = sample
        self._trigger   = trigger
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
    def _update_trigger_project(self) -> tuple[Trigger,str]:
        '''
        Returns
        -------------
        Tuple with trigger and project. If trigger does not end with ext
        return _trigger and default project, i.e. rk, rkst 

        If trigger ends with ext, switch to noPID trigger and rk/rkst_nopid project
        because we are working with PID control region with simulation.
        '''
        default_project = info.project_from_trigger(trigger = self._trigger, lower_case=True)
        if self._trigger not in [Trigger.rk_ee_ext, Trigger.rkst_ee_ext]:
            log.debug(f'Found non-Extended trigger {self._trigger}, using default project {default_project}')
            return self._trigger, default_project

        value   = self._trigger.replace('_ext', '_noPID')
        trigger = Trigger(value)
        project = f'{default_project}_nopid'

        log.info(f'Found ext trigger, overriding with: {trigger}/{project}')

        return trigger, project
    # ------------------------
    def run(self) -> ExtendedUnbinnedNLL:
        '''
        Runs fit

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

        trigger, project = self._update_trigger_project()

        log.info('Getting full data model using fits to simulation')
        with SpecMaker.project(name = project):
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
