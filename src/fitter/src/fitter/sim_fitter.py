'''
Module with SimFitter class
'''
from typing                   import overload, Literal, Final

from dmu                      import LogStore
from dmu.stats                import FitResult, ModelFactoryConf
from dmu.stats                import ModelFactory
from dmu.stats                import zfit
from dmu.stats                import utilities        as sut
from dmu.workflow             import Cache

from rx_common                import Qsq, Trigger, Component
from rx_selection             import selection        as sel

from zfit.data                import Data             as zdata
from zfit.pdf                 import BasePDF          as zpdf
from zfit                     import Space            as zobs
from zfit.param               import Parameter

from .base_fitter             import BaseFitter
from .configs                 import CombinatorialConf, NonParametricConf, ParametricConf, CCbarConf 
from .data_preprocessor       import DataPreprocessor
from .prec                    import PRec

log=LogStore.add_logger('fitter:sim_fitter')

ModelConf = CombinatorialConf | ParametricConf | NonParametricConf
MAIN_CATEGORY   : Final[str] = 'main'

# Will not build (fit) a parametric PDF if fewer than these entries
# will return None
MIN_FIT_ENTRIES : Final[int] = 50

# Will not build KDE if fewer than these entries in dataset
MIN_KDE_ENTRIES : Final[int] = 50
# ------------------------
class SimFitter(BaseFitter, Cache):
    '''
    Fitter for simulation samples
    '''
    # ------------------------
    def __init__(
        self,
        component : Component,
        trigger   : Trigger,
        q2bin     : Qsq,
        cfg       : ModelConf,
        obs       : zobs,
        name      : str):
        '''
        Parameters
        --------------------
        obs      : Observable
        name     : Optional, identifier for fit, used to name directory
        component: Nickname of component, e.g. combinatorial, only used for naming
        trigger  : Hlt2RD...
        q2bin    : E.g. central
        cfg      : Object storing configuration for fit
        '''
        BaseFitter.__init__(self)
        log.info(20 * '-')
        log.info(f'Fitting {component}/{name}')
        log.info(20 * '-')

        self._component = component
        self._trigger   = trigger
        self._q2bin     = q2bin
        self._name      = name
        self._cfg       = cfg
        self._obs       = obs
        self._base_path = cfg.output_directory / f'{name}/{trigger}_{q2bin}'

        log.debug(f'For component {self._component} using output: {self._base_path}')

        self._l_rdf_uid = []
        self._d_data    = self._get_data()

        # All the PDFs will share the mu and sigma below and these will float
        self._mu_par = Parameter('mu_flt', 5280, 5000, 5500)
        self._sg_par = Parameter('sg_flt',   15,    5,  300)

        Cache.__init__(
            self,
            out_path = self._base_path,
            l_rdf_uid= self._l_rdf_uid,
            config   = self._cfg.model_dump())
    # ------------------------
    # Data getting section
    # ------------------------
    def _get_data(self) -> dict[str,zdata]:
        '''
        Returns
        --------------------
        dictionary with:

        Key  : Name of MC category, e.g. brem category
        Value: Zfit dataset
        '''
        if isinstance(self._cfg, CombinatorialConf):
            return dict() 

        if isinstance(self._cfg, ParametricConf):
            return self._get_category_data(cfg = self._cfg)

        if isinstance(self._cfg, NonParametricConf):
            return self._get_nonparametric_data(cfg = self._cfg)

        raise ValueError(f'Cannot get data for: {type(self._cfg)}')
    # ------------------------
    def _get_category_data(self, cfg : ParametricConf) -> dict[str,zdata]:
        '''
        Parameters
        --------------
        cfg: Object storing configuration
        '''
        d_data = {}
        for cat_name, cat_cfg in cfg.categories.items():
            prp   = DataPreprocessor(
                wgt_cfg = dict(),
                obs     = self._obs,
                cut     = cat_cfg.selection,
                trigger = self._trigger,
                q2bin   = self._q2bin,
                out_dir = self._base_path,
                sample  = cfg.sample)

            d_data[cat_name] = prp.get_data()

            self._l_rdf_uid.append(prp.rdf_uid)

        return d_data
    # ------------------------
    def _get_nonparametric_data(self, cfg : NonParametricConf) -> dict[str,zdata]:
        '''
        Parameters
        --------------
        cfg: Object storing configuration
        '''
        d_data = {}
        prp   = DataPreprocessor(
            wgt_cfg = dict(),
            obs     = self._obs,
            cut     = dict(),
            trigger = self._trigger,
            q2bin   = self._q2bin,
            out_dir = self._base_path,
            sample  = cfg.sample)

        d_data['main'] = prp.get_data()

        self._l_rdf_uid.append(prp.rdf_uid)

        return d_data
    # ------------------------
    # PDF getting section
    # ------------------------
    def _get_pdf(
        self,
        cfg     : ModelFactoryConf,
        category: str) -> zpdf:
        '''
        Parameters
        ------------
        category: If the MC is meant to be split (e.g. by brem) this should the the label of the category
        cfg     : Configuration needed to build model

        Returns
        ------------
        Fitting PDF built from the sum of those models
        '''
        log.info(f'Building {self._component} for category {category} with: {cfg.pdfs}')

        mod         = ModelFactory(
            preffix = self._get_suffix(category=category),
            obs     = self._obs,
            l_pdf   = cfg.pdfs,
            l_reuse = cfg.reuse,
            l_shared= cfg.shared,
            l_float = cfg.floating ,
            d_rep   = cfg.reparametrize,
            d_fix   = cfg.fix)

        pdf = mod.get_pdf()

        return pdf
    # ----------------------
    def _get_suffix(self, category : str) -> str:
        '''
        Parameters
        -------------
        category: Name of model category 

        Returns
        -------------
        Name of suffix for naming parameters
        '''
        if self._name is None:
            return f'{self._component}_{category}'

        return f'{self._component}_{category}_{self._name}'
    # ------------------------
    def _fix_tails(self, pdf : zpdf, res : FitResult) -> zpdf:
        '''
        Parameters
        --------------
        pdf : PDF after fit
        res : Fit result object, used to fix tails

        Returns
        --------------
        PDF with tails fixed
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

            if par.name in res:
                val, _ = res[par.name]

                par.set_value(val)
                log.debug(f'{par.name:<20}{"--->"}{val:>20.3f}')
                par.floating = False

        return pdf
    # ------------------------
    def _get_nomc_pdf(
        self,
        cfg : CombinatorialConf) -> zpdf:
        '''
        Returns
        -------------
        PDF for component that does not require MC, e.g. combinatorial
        '''

        model_cfg = cfg.models[self._q2bin]
        model     = self._get_pdf(
            cfg     = model_cfg,
            category= 'main')

        return model
    # ------------------------
    # Fitting section
    # ------------------------
    def _fit_category(
        self,
        cfg      : ParametricConf,
        skip_fit : bool,
        category : str) -> tuple[zpdf|None,float|None,FitResult|None]:
        '''
        Parameters
        ----------------
        skip_fit     : If true, it will only return model, used if fit parameters were already found
        category     : Name of fitting category

        Returns
        ----------------
        Tuple with:
            - Fitted PDF, None if problems were found building it, e.g. too few entries
            - Size (sum of weights) of dataset in given category.
              If fit is skipped, returns None, because this is used to set
              the value of the fit fraction, which should already be in the cached data.
            - Fit result object, if fit is skipped, returns None
        '''
        log.info(f'Fitting category {category}')

        cat_cfg = cfg.categories[category]
        model = self._get_pdf(
            category= category,
            cfg     = cat_cfg.model)

        data  = self._d_data[category]

        sumw  = sut.yield_from_zdata(data=data)
        if skip_fit:
            return model, sumw, None

        cut_cfg = self._get_cut_config(cfg = cfg, category = category)

        if sumw < MIN_FIT_ENTRIES:
            log.warning(f'Found to few entries {sumw:.1f} < {MIN_FIT_ENTRIES}, skipping {self._component} component')
            self._save_fit(
                cut_cfg  = cut_cfg,
                plt_cfg  = cfg.plots,
                data     = data,
                model    = None,
                res      = None,
                out_path = self._out_path / category )

            return None, 0, None

        res = self._fit(
            data = data, 
            model= model, 
            cfg  = cfg.fit)

        self._save_fit(
            cut_cfg  = cut_cfg,
            plt_cfg  = cfg.plots,
            data     = data,
            model    = model,
            res      = res,
            out_path = self._out_path / category)

        model = self._fix_tails(pdf=model, res=res)

        return model, sumw, res
    # ----------------------
    def _get_cut_config(
        self, 
        category : str | None,
        cfg      : ParametricConf | NonParametricConf) -> dict[str,dict[str,str]]:
        '''
        Parameters
        -------------
        category: Fit category, e.g. brem_xx1 or None, if it does not make sense, e.g. KDE fits

        Returns
        -------------
        Dictionary with:
            Keys  : `fit`, `default`
            Values: Selection for data that was fitted and default
        '''
        cuts_current= sel.selection(
            process = cfg.sample, 
            trigger = self._trigger, 
            q2bin   = self._q2bin)

        if isinstance(cfg, ParametricConf) and category is not None:
            fit_cuts = cfg.categories[category].selection
            cuts_current.update(fit_cuts)

        cuts = {'fit' : cuts_current}

        with sel.custom_selection(d_sel={}, force_override=True):
            cuts['default'] = sel.selection(
                process = cfg.sample, 
                trigger = self._trigger, 
                q2bin   = self._q2bin)

        return cuts
    # ------------------------
    # TODO: Fractions need to be parameters to be constrained
    def _get_fraction(
        self,
        sumw     : float,
        total    : float,
        category : str) -> Parameter:
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
        frac_name = f'frac_{self._component}_{category}'
        value     = sumw / total
        par       = Parameter(frac_name, value, 0, 1)

        log.debug(f'{frac_name:<50}{value:<10.3f}')

        return par
    # ------------------------
    @overload
    def _get_full_model(self, cfg : ParametricConf, skip_fit : Literal[True]) -> zpdf: ...
    @overload
    def _get_full_model(self, cfg : ParametricConf, skip_fit : Literal[False]) -> tuple[zpdf,FitResult] | None: ...
    def _get_full_model(self, cfg : ParametricConf, skip_fit : bool) -> tuple[zpdf,FitResult] | zpdf | None:
        '''
        Parameters
        ---------------
        cfg     : Configuration for fits to parametric PDFs
        skip_fit: If true, it will rturn the model without fitting

        Returns
        ---------------
        Tuple with:

        - PDF for the combined categories with the parameters set
        to the fitted values
        - Instance of FitResult
        '''
        l_pdf   : list[zpdf      ] = []
        l_yield : list[float     ] = []
        l_res   : list[FitResult ] = []
        for category in cfg.categories:
            model, sumw, res = self._fit_category(
                cfg          = cfg,
                skip_fit     = skip_fit,
                category     = category)

            if model is None or res is None or sumw is None:
                log.warning(f'Skipping category {category}')
                continue

            l_pdf.append(model)
            l_yield.append(sumw)
            l_res.append(res)

        if len(l_pdf) == 0:
            return None

        return self._merge_categories(
            cfg    = cfg,
            l_pdf  = l_pdf,
            l_yield= l_yield,
            l_res  = l_res)
    # ------------------------
    def _merge_categories(
        self,
        cfg     : ParametricConf,
        l_pdf   : list[zpdf],
        l_yield : list[float],
        l_res   : list[FitResult]) -> tuple[zpdf,FitResult]:
        '''
        Parameters
        -----------------
        l_pdf  : List of zfit PDFs from fit, one per category
        l_yield: List of yields from MC sample, not the fitted one
        l_res  : List of fit result objects holding parameter values from fits

        Returns
        -----------------
        Tuple with:

        - Full PDF, i.e. sum of components
        - Merged dictionary of parameters
        '''
        if len(l_pdf) == 1:
            return l_pdf[0], l_res[0]

        log.debug(60 * '-')
        log.debug(f'{"Fraction":<50}{"Value":<10}')
        log.debug(60 * '-')
        l_frac = [
            self._get_fraction(
                sumw,
                total   = sum(l_yield),
                category= category)
            for sumw, category in zip(l_yield, cfg.categories, strict=True) ]
        log.debug(60 * '-')

        full_model = zfit.pdf.SumPDF(l_pdf, l_frac)
        full_res   = FitResult.merge(results = l_res)

        return full_model, full_res
    # ------------------------
    def _get_kde(self, cfg : NonParametricConf) -> zpdf | None:
        '''
        KDE is not fit in categories, there is just a MAIN_CATEGORY

        - Makes KDE PDF
        - Saves fit (plot, list of parameters, etc)

        Parameters
        ------------------
        cfg: Configuration for KDE

        Returns
        ------------------
        - KDE PDF after fit
        - None if there are fewer than _min_kde_entries
        '''
        data = self._d_data[MAIN_CATEGORY]

        if data.nevents < MIN_KDE_ENTRIES:
            log.info(f'Not bulding KDE, found too few entries: {data.nevents} < {MIN_KDE_ENTRIES}')
            return 

        kde_builder = getattr(zfit.pdf, cfg.fit.kind)

        pdf = kde_builder(
            obs       = self._obs, 
            data      = data, 
            name      = self._component, 
            bandwidth = cfg.fit.bandwidth,
            padding   = cfg.fit.padding)

        self._save_fit(
            cut_cfg  = self._get_cut_config(cfg = cfg),
            plt_cfg  = cfg.plots,
            data     = data,
            model    = pdf,
            res      = None,
            out_path = self._out_path / MAIN_CATEGORY)

        return pdf
    # ------------------------
    def _get_ccbar_component(
        self,
        cfg : CCbarConf) -> zpdf | None:
        '''
        This is an interace to the PRec class, which is in
        charge of building the KDE for ccbar sample

        Parameters
        -------------------
        cfg: Configuration

        Returns
        ----------------
        Either:

        - PDF with KDEs added corresponding to different groups of ccbar decays
        - None, if no data was found
        '''
        ftr=PRec(
            obs     = self._obs,
            trig    = self._trigger,
            q2bin   = self._q2bin  ,
            cfg     = cfg)

        pdf =ftr.get_sum(name = r'$c\bar{c}+X$')

        return pdf
    # ------------------------
    def get_model(self) -> zpdf|None:
        '''
        Returns
        ------------
        Either:

        - zfit PDF, not extended yet
        - None, if statistics are too low to build PDF
        '''
        if isinstance(self._cfg, CCbarConf):
            return self._get_ccbar_component(cfg = self._cfg)

        if isinstance(self._cfg, CombinatorialConf):
            return self._get_nomc_pdf(cfg = self._cfg)

        if isinstance(self._cfg, NonParametricConf):
            return self._get_kde(cfg = self._cfg)

        if not isinstance(self._cfg, ParametricConf):
            raise ValueError(f'Config not a parametric one: {type(self._cfg)}')

        result_path = self._out_path / 'parameters.yaml'
        if self._copy_from_cache():
            res = FitResult.from_json(path = result_path)

            # If caching, need only model, second return value
            # Is None because no fit happened 
            model = self._get_full_model(skip_fit=True, cfg = self._cfg)
            model = self._fix_tails(pdf=model, res=res)

            return model

        log.info(f'Fitting, could not find cached parameters in {result_path}')

        val = self._get_full_model(skip_fit=False, cfg = self._cfg)
        if val is None:
            return None

        full_model, fres = val

        fres.to_json(path = result_path)

        self._cache()
        return full_model
# ------------------------
