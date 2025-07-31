'''
Module containing DataFitter class
'''
from typing import cast

from omegaconf                import DictConfig, OmegaConf

from dmu.workflow.cache       import Cache
from dmu.stats.zfit           import zfit
from dmu.stats                import utilities  as sut
from dmu.logging.log_store    import LogStore
from rx_selection             import selection  as sel

from zfit.interface           import ZfitPDF    as zpdf
from zfit.interface           import ZfitSpace  as zobs
from fitter.data_preprocessor import DataPreprocessor
from fitter.base_fitter       import BaseFitter
from fitter.data_model        import DataModel
from fitter.constraint_reader import ConstraintReader

log=LogStore.add_logger('fitter:data_fitter')
# ------------------------
class DataFitter(BaseFitter, Cache):
    '''
    Fitter for data
    '''
    # ------------------------
    def __init__(
            self,
            sample  : str,
            trigger : str,
            project : str,
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
        BaseFitter.__init__(self)

        self._sample    = sample
        self._trigger   = trigger
        self._project   = project
        self._q2bin     = q2bin
        self._cfg       = cfg
        self._name      = name
        self._base_path = self._get_base_path()

        Cache.__init__(
            self,
            out_path = self._base_path,
            cuts     = sel.selection(process=sample, trigger=trigger, q2bin=q2bin),
            config   = OmegaConf.to_container(cfg, resolve=True))

        self._obs = self._make_observable()
    # ------------------------
    def _get_base_path(self) -> str:
        '''
        Returns directory where outputs will go
        '''
        sample = self._sample.replace('*', 'p')
        if self._name is not None:
            sample = f'{self._cfg.output_directory}/{sample}/{self._name}/{self._trigger}_{self._project}_{self._q2bin}'
        else:
            sample = f'{self._cfg.output_directory}/{sample}/{self._trigger}_{self._project}_{self._q2bin}'

        return sample
    # ------------------------
    def _make_observable(self) -> zobs:
        '''
        Will return zfit observable
        '''
        name        = self._cfg.model.observable.name
        [minx, maxx]= self._cfg.model.observable.range

        return zfit.Space(name, limits=(minx, maxx))
    # ------------------------
    def _constraints_from_model(self, model : zpdf) -> dict[str,tuple[float,float]]:
        '''
        Parameters
        ----------------
        model: Model needed to fit the data

        Returns
        ----------------
        Dictionary with:

        Key: Name of parameter
        Value: Tuple value, error. Needed to apply constraints
        '''
        log.info('Getting constraints')

        s_par   = model.get_params()
        l_par   = [ par.name for par in s_par ]
        obj     = ConstraintReader(parameters = l_par, q2bin=self._q2bin)
        d_cns   = obj.get_constraints()

        log.debug(90 * '-')
        log.debug(f'{"Name":<20}{"Value":<20}{"Error":<20}')
        log.debug(90 * '-')
        for name, (val, err) in d_cns.items():
            log.debug(f'{name:<50}{val:<20.3f}{err:<20.3f}')
        log.debug(90 * '-')

        return d_cns
    # ----------------------
    def _update_trigger_project(self) -> tuple[str,str]:
        '''
        Returns
        -------------
        Tuple with trigger and project. If trigger does not end with ext
        return _trigger and _project

        If trigger ends with ext, switch to noPID trigger and nopid project
        because we are working with PID control region.
        '''
        if not self._trigger.endswith('_ext'):
            return self._trigger, self._project

        trigger = self._trigger.replace('_ext', '_noPID')
        project = 'nopid'

        log.info('Found ext trigger, overriding with: {trigger}/{project}')

        return trigger, project
    # ------------------------
    def run(self) -> DictConfig:
        '''
        Runs fit

        Returns
        ------------
        DictConfig object with fitting results
        '''

        result_path = f'{self._out_path}/parameters.yaml'
        if self._copy_from_cache():
            res = OmegaConf.load(result_path)
            res = cast(DictConfig, res)

            return res

        dpr  = DataPreprocessor(
            obs    = self._obs,
            q2bin  = self._q2bin,
            sample = self._sample,
            trigger= self._trigger,
            out_dir= self._base_path,
            project= self._project,
            wgt_cfg= None) # Do not need weights for data
        data = dpr.get_data()

        trigger, project = self._update_trigger_project()

        mod  = DataModel(
            name   = self._name,
            cfg    = self._cfg,
            obs    = self._obs,
            q2bin  = self._q2bin,
            trigger= trigger,
            project= project)
        model= mod.get_model()
        d_cns= self._constraints_from_model(model=model)

        res  = self._fit(data=data, model=model, d_cns=d_cns, cfg=self._cfg.fit)
        self._save_fit(
            cuts     = sel.selection(process=self._sample, trigger=self._trigger, q2bin=self._q2bin),
            cfg      = self._cfg.plots,
            data     = data,
            model    = model,
            res      = res,
            d_cns    = d_cns,
            out_path = self._out_path)

        cres = sut.zres_to_cres(res=res)

        self._cache()

        return cres
# ------------------------
