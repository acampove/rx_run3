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
    def _fit_category(
            self,
            skip_fit     : bool,
            category     : str,
            l_model_name : list[str]) -> tuple[zpdf,float|None,zres|None]:
        '''
        Parameters
        ----------------
        skip_fit     : If true, it will only return model, used if fit parameters were already found
        category     : Name of fitting category
        l_model_name : List of fitting models,  e.g. [cbr, cbl]

        Returns
        ----------------
        Tuple with:
            - fitted PDF
            - size (sum of weights) of dataset in given category.
              If fit is skipped, returns None, because this is used to set
              the value of the fit fraction, which should already be in the cached data.
            - zfit result object, if fit is skipped, returns None
        '''
        log.info(f'Fitting category {category}')

        model = self._get_pdf(category=category, l_model=l_model_name)
        data  = self._d_data[category]

        sumw= data.weights.numpy().sum()
        if skip_fit:
            return model, sumw, None

        res   = self._fit(data=data, model=model, cfg=self._cfg.fit)
        self._save_fit(
            cuts     = sel.selection(process=self._cfg.sample, trigger=self._trigger, q2bin=self._q2bin),
            cfg      = self._cfg.plots,
            data     = data,
            model    = model,
            res      = res,
            out_path = f'{self._out_path}/{category}')

        cres  = sut.zres_to_cres(res=res)
        model = self._fix_tails(pdf=model, pars=cres)

        return model, sumw, res
    # ------------------------
    # TODO: Fractions need to be parameters to be constrained
    def _get_fraction(
        self,
        sumw     : float,
        total    : float,
        category : str) -> zpar:
        '''
        Parameters
        -------------
        sumw    : Yield in MC associated to this category
        total   : Total yield
        category: Name of this category

        Returns
        -------------
        Fitting fraction parameter fixed
        '''
        frac_name = f'frac_{self._name}_{category}'
        value     = sumw / total
        par       = zfit.param.Parameter(frac_name, value, 0, 1)

        log.debug(f'{frac_name:<50}{value:<10.3f}')

        return par
    # ------------------------
    def _get_full_model(self, skip_fit : bool) -> tuple[zpdf,DictConfig]:
        '''
        Parameters
        ---------------
        skip_fit: If true, it will rturn the model without fitting

        Returns
        ---------------
        Tuple with:

        - PDF for the combined categories with the parameters set
        to the fitted values
        - Instance of DictConfig storing all the fitting parameters
        '''
        l_pdf   = []
        l_yield = []
        l_cres  = []
        for category, data in self._cfg.categories.items():
            l_model_name     = data['model']
            model, sumw, res = self._fit_category(
                skip_fit     = skip_fit,
                category     = category,
                l_model_name = l_model_name)

            # Will be None if fit is cached
            # and this is only returning model
            cres = OmegaConf.create({})
            if res is not None:
                cres = sut.zres_to_cres(res)

            l_pdf.append(model)
            l_yield.append(sumw)
            l_cres.append(cres)

        return self._merge_categories(
            l_pdf  =l_pdf,
            l_yield=l_yield,
            l_cres =l_cres)
    # ------------------------
    def _merge_categories(
        self, 
        l_pdf   : list[zpdf], 
        l_yield : list[float],
        l_cres  : list[DictConfig]) -> tuple[zpdf,DictConfig]:
        '''
        Parameters
        -----------------
        l_pdf  : List of zfit PDFs from fit, one per category
        l_yield: List of yields from MC sample, not the fitted one
        l_cres : List of result objects holding parameter values from fits

        Returns
        -----------------
        Tuple with:

        - Full PDF, i.e. sum of components
        - Merged dictionary of parameters
        '''

        if len(l_pdf) == 1:
            cres  = OmegaConf.merge(l_cres[0])

            return l_pdf[0], cres

        log.debug(60 * '-')
        log.debug(f'{"Fraction":<50}{"Value":<10}')
        log.debug(60 * '-')
        l_frac = [
                  self._get_fraction(
                      sumw,
                      total   = sum(l_yield),
                      category= category)
                  for sumw, category in zip(l_yield, self._cfg.categories) ]
        log.debug(60 * '-')

        full_model = zfit.pdf.SumPDF(l_pdf, l_frac)
        full_cres  = OmegaConf.merge(*l_cres)

        return full_model, full_cres
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
            res      = OmegaConf.load(result_path)
            res      = cast(DictConfig, res)
            # If caching, need only model, second return value
            # Is an empty DictConfig, because no fit happened
            model, _ = self._get_full_model(skip_fit=True)
            model    = self._fix_tails(pdf=model, pars=res)

            return model

        log.info(f'Fitting, could not find cached parameters in {result_path}')

        full_model, cres = self._get_full_model(skip_fit=False)

        OmegaConf.save(cres, result_path)

        self._cache()
        return full_model
# ------------------------
