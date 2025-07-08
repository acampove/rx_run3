'''
Module with SimFitter class
'''

from typing import cast

from omegaconf                import DictConfig, OmegaConf

from dmu.stats.zfit           import zfit
from dmu.stats                import utilities    as sut
from dmu.workflow.cache       import Cache
from dmu.stats.model_factory  import ModelFactory
from dmu.logging.log_store    import LogStore

from rx_selection             import selection    as sel
from zfit.core.interfaces     import ZfitData     as zdata
from zfit.core.interfaces     import ZfitPDF      as zpdf
from zfit.core.parameter      import Parameter    as zpar
from zfit.result              import FitResult    as zres
from zfit.core.interfaces     import ZfitSpace    as zobs
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
        self._name      = name
        self._trigger   = trigger
        self._project   = project
        self._q2bin     = q2bin
        self._cfg       = cfg
        self._obs       = obs
        self._base_path = f'{cfg.output_directory}/{name}_{trigger}_{project}_{q2bin}'
        self._l_rdf_uid = []
        self._d_data    = self._get_data()

        BaseFitter.__init__(self)
        Cache.__init__(
            self,
            out_path = self._base_path,
            l_rdf_uid= self._l_rdf_uid,
            config   = OmegaConf.to_container(cfg, resolve=True))
    # ------------------------
    def _get_data(self) -> dict[str,zdata]:
        '''
        Returns
        --------------------
        dictionary with:

        Key  : Name of MC category, e.g. brem category
        Value: Zfit dataset
        '''
        d_data = {}

        for cat_name, data in self._cfg.categories.items():
            cat_cut = None if 'selection' not in data else data.selection

            prp   = DataPreprocessor(
                obs    = self._obs,
                cut    = cat_cut,
                trigger= self._trigger,
                project= self._project,
                q2bin  = self._q2bin,
                out_dir= self._base_path,
                sample = self._cfg.sample)
            d_data[cat_name] = prp.get_data()

            self._l_rdf_uid.append(prp.rdf_uid)

        return d_data
    # ------------------------
    def _get_pdf(
            self,
            category: str,
            l_model : list[str]) -> zpdf:
        '''
        Parameters
        ------------
        category: If the MC is meant to be split (e.g. by brem) this should the the label of the category
        l_model : List of model names, e.g. [cbl, cbr]

        Returns
        ------------
        Fitting PDF built from the sum of those models
        '''
        mod     = ModelFactory(
            preffix = f'{self._name}_{category}',
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
        npar  = len(s_par)
        log.debug(f'Found {npar} floating parameters')

        for par in s_par:
            # Model builder adds _flt to name
            # of parameters meant to float
            if par.name.endswith('_flt'):
                log.debug(f'Not fixing: {par.name}')
                continue

            if par.name in pars:
                par.set_value(pars[par.name].value)
                log.debug(f'{par.name:<20}{"--->"}{pars[par.name].value:>20.3f}')
                par.floating = False

        return pdf
    # ------------------------
    def _get_nomc_component(self) -> zpdf:
        '''
        This method will return a PDF when there is no simulation
        associated to it, e.g. Combinatorial
        '''
        if 'main' not in self._cfg.categories:
            log.info(OmegaConf.to_yaml(self._cfg))
            raise ValueError(f'Cannot find main category in config associated to sample {self._name}')

        l_model = self._cfg.categories.main.models
        model   = self._get_pdf(l_model=l_model, category='main')

        return model
    # ------------------------
    def get_model(self) -> zpdf:
        '''
        Returns
        ------------
        zfit PDF, not extended yet
        '''
        if 'sample' not in self._cfg:
            return self._get_nomc_component()

        result_path = f'{self._out_path}/parameters.yaml'
        if self._copy_from_cache():
            res   = OmegaConf.load(result_path)
            res   = cast(DictConfig, res)
            model = self._fix_tails(pdf=model, pars=res)

            return model


        res   = self._fit(data=data, model=model, cfg=self._cfg.fit)
        self._save_fit(
            cuts     = sel.selection(process=self._cfg.sample, trigger=self._trigger, q2bin=self._q2bin),
            cfg      = self._cfg.plots,
            data     = data,
            model    = model,
            res      = res,
            out_path = self._out_path)

        res   = sut.zres_to_cres(res=res)
        model = self._fix_tails(pdf=model, pars=res)

        self._cache()

        return model
# ------------------------
