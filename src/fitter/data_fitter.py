'''
Module containing DataFitter class
'''
from typing import cast

from omegaconf                import DictConfig, OmegaConf

from dmu.workflow.cache       import Cache
from dmu.stats.zfit           import zfit
from dmu.stats.zfit_plotter   import ZFitPlotter
from dmu.stats                import utilities  as sut
from dmu.logging.log_store    import LogStore

from zfit.core.interfaces     import ZfitSpace  as zobs
from fitter.data_preprocessor import DataPreprocessor
from fitter.base_fitter       import BaseFitter
from fitter.data_model        import DataModel

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
            cfg     : DictConfig):
        '''
        cfg    : configuration for the fit as a DictConfig object
        sample : Identifies sample e.g. DATA_24_MagUp...
        trigger: Hlt2RD...
        project: E.g. rx
        q2bin  : E.g. central
        cfg    : Configuration for the fit to data
        '''
        self._sample = sample
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._cfg    = cfg

        BaseFitter.__init__(self)
        Cache.__init__(
                self,
                out_path = cfg.output_directory,
                config   = OmegaConf.to_container(cfg, resolve=True))

        self._obs = self._make_observable()
    # ------------------------
    def _make_observable(self) -> zobs:
        '''
        Will return zfit observable
        '''
        name        = self._cfg.fit.observable.name
        [minx, maxx]= self._cfg.fit.observable.range

        return zfit.Space(name, limits=(minx, maxx))
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
            project= self._project)
        data = dpr.get_data()

        mod  = DataModel(
            cfg    = self._cfg,
            obs    = self._obs,
            q2bin  = self._q2bin,
            sample = self._sample,
            trigger= self._trigger,
            project= self._project)
        model= mod.get_model()

        res  = self._fit(data=data, model=model)

        ptr = ZFitPlotter(data=data, model=model)
        ptr.plot(**self._cfg.plots)

        sut.save_fit(
            data   = data,
            model  = model,
            res    = res,
            fit_dir= self._out_path)

        self._cache()
        cres = sut.zres_to_cres(res=res)

        return cres
# ------------------------
