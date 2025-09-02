'''
Module with SimFitter class
'''

from typing import cast

import yaml
from omegaconf                import DictConfig, OmegaConf

from dmu.stats.zfit           import zfit
from dmu.stats                import utilities    as sut
from dmu.workflow.cache       import Cache
from dmu.stats.model_factory  import ModelFactory
from dmu.logging.log_store    import LogStore

from rx_efficiencies.decay_names import DecayNames
from rx_selection             import selection        as sel
from zfit.data                import Data             as zdata
from zfit.pdf                 import BasePDF          as zpdf
from zfit.param               import Parameter
from zfit.interface           import ZfitSpace        as zobs
from zfit.result              import FitResult        as zres
from fitter.base_fitter       import BaseFitter
from fitter.data_preprocessor import DataPreprocessor
from fitter.prec              import PRec

log=LogStore.add_logger('fitter:sim_fitter')
# ------------------------
class SimFitter(BaseFitter, Cache):
    '''
    Fitter for simulation samples
    '''
    # ------------------------
    def __init__(
        self,
        component : str,
        trigger   : str,
        project   : str,
        q2bin     : str,
        cfg       : DictConfig,
        obs       : zobs,
        name      : str|None = None):
        '''
        Parameters
        --------------------
        obs      : Observable
        name     : Optional, identifier for fit, used to name directory
        component: Nickname of component, e.g. combinatorial, only used for naming
        trigger  : Hlt2RD...
        project  : E.g. rx
        q2bin    : E.g. central
        cfg      : Object storing configuration for fit
        '''
        BaseFitter.__init__(self)

        self._sample    = DecayNames.sample_from_decay(component, fall_back='NA')
        self._name      = name
        self._component = component
        self._trigger   = trigger
        self._project   = project
        self._q2bin     = q2bin
        self._cfg       = cfg
        self._obs       = obs
        if name is None:
            self._base_path = f'{cfg.output_directory}/{trigger}_{project}_{q2bin}'
        else:
            self._base_path = f'{cfg.output_directory}/{name}/{trigger}_{project}_{q2bin}'

        log.debug(f'For component {self._component} using output: {self._base_path}')

        self._l_rdf_uid = []
        self._d_data    = self._get_data()

        # Will not build KDE if fewer than these entries in dataset
        self._min_kde_entries = 50

        # Will not build (fit) a parametric PDF if fewer than these entries
        # will return None
        self._min_fit_entries = 100

        # All the PDFs will share the mu and sigma below and these will float
        self._mu_par = Parameter('mu_flt', 5280, 5000, 5500)
        self._sg_par = Parameter('sg_flt',   15,    5,  300)

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
        # For components without an MC associated e.g. combinatorial
        # return empty dataset
        if 'sample' not in self._cfg:
            return d_data

        for cat_name, cat_cfg in self._cfg.categories.items():
            prp   = DataPreprocessor(
                obs    = self._obs,
                cut    = cat_cfg.get('selection'),
                wgt_cfg= cat_cfg.get('weights'),
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
        cfg     : DictConfig,
        category: str,
        l_model : list[str]) -> zpdf:
        '''
        Parameters
        ------------
        category: If the MC is meant to be split (e.g. by brem) this should the the label of the category
        cfg     : DictConfig with model configuration, stores parameters that are floating, fixed, etc
        l_model : List of model names, e.g. [cbl, cbr]

        Returns
        ------------
        Fitting PDF built from the sum of those models
        '''
        log.info(f'Building {self._component} for category {category} with: {l_model}')

        mod         = ModelFactory(
            preffix = self._get_suffix(category=category),
            obs     = self._obs,
            l_pdf   = l_model,
            l_reuse = [self._mu_par, self._sg_par],
            l_shared= cfg.shared,
            l_float = cfg.float ,
            d_rep   = cfg.reparametrize,
            d_fix   = cfg.fix)

        pdf = mod.get_pdf()

        return pdf
    # ----------------------
    def _get_suffix(self, category : str) -> str:
        '''
        Parameters
        -------------
        category: Usually `main` otherwise could be brem0, etc

        Returns
        -------------
        Name of suffix for naming parameters
        '''
        if self._name is None:
            return f'{self._component}_{category}'

        return f'{self._component}_{category}_{self._name}'
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
            raise ValueError(f'Cannot find main category in config associated to sample {self._component}')

        l_model = self._cfg.categories.main.models[self._q2bin]
        cfg     = self._cfg[self._q2bin]

        model   = self._get_pdf(
            l_model = l_model,
            cfg     = cfg,
            category= 'main')

        return model
    # ------------------------
    def _fit_category(
            self,
            skip_fit     : bool,
            category     : str,
            l_model_name : list[str]) -> tuple[zpdf|None,float|None,zres|None]:
        '''
        Parameters
        ----------------
        skip_fit     : If true, it will only return model, used if fit parameters were already found
        category     : Name of fitting category
        l_model_name : List of fitting models,  e.g. [cbr, cbl]

        Returns
        ----------------
        Tuple with:
            - Fitted PDF, None if problems were found building it, e.g. too few entries
            - Size (sum of weights) of dataset in given category.
              If fit is skipped, returns None, because this is used to set
              the value of the fit fraction, which should already be in the cached data.
            - zfit result object, if fit is skipped, returns None
        '''
        log.info(f'Fitting category {category}')

        model = self._get_pdf(
            category= category,
            cfg     = self._cfg,
            l_model = l_model_name)

        data  = self._d_data[category]

        sumw  = sut.yield_from_zdata(data=data)
        if skip_fit:
            return model, sumw, None

        if sumw < self._min_fit_entries:
            log.warning(f'Found to few entries {sumw:.1f} < {self._min_fit_entries}, skipping component')
            return None, 0, None

        res   = self._fit(data=data, model=model, cfg=self._cfg.fit)

        self._save_fit(
            cut_cfg  = self._get_cut_config(),
            plt_cfg  = self._cfg.plots,
            data     = data,
            model    = model,
            res      = res,
            out_path = f'{self._out_path}/{category}')

        cres  = sut.zres_to_cres(res=res)
        model = self._fix_tails(pdf=model, pars=cres)

        return model, sumw, res
    # ----------------------
    def _get_cut_config(self) -> DictConfig:
        '''
        Returns
        -------------
        Dictionary with:
            Keys  : `fit`, `default`
            Values: Selection for data that was fitted and default
        '''
        cfg        = {}
        cfg['fit'] = sel.selection(process=self._cfg.sample, trigger=self._trigger, q2bin=self._q2bin)
        with sel.custom_selection(d_sel={}, force_override=True):
            cfg['default'] = sel.selection(process=self._cfg.sample, trigger=self._trigger, q2bin=self._q2bin)

        return OmegaConf.create(cfg)
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
    def _get_full_model(self, skip_fit : bool) -> tuple[zpdf,DictConfig]|None:
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

            if model is None:
                log.warning(f'Skipping cateogory {category}')
                continue

            # Will be None if fit is cached
            # and this is only returning model
            cres = OmegaConf.create({})
            if res is not None:
                cres = sut.zres_to_cres(res)

            l_pdf.append(model)
            l_yield.append(sumw)
            l_cres.append(cres)

        if len(l_pdf) == 0:
            return None

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
            if not isinstance(cres, DictConfig):
                raise TypeError('Merged parameters are not a DictConfig')

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
        if not isinstance(full_cres, DictConfig):
            raise TypeError('Merged dictionary not a DictConfig')

        return full_model, full_cres
    # ------------------------
    def _is_kde(self) -> bool:
        '''
        Returns true if the PDF is meant to be a single KDE
        False if it is meant to be a parametric PDF
        '''
        if len(self._cfg.categories) > 1:
            return False

        # By convention, if there is a single category
        # It will be the main category
        model_name = self._cfg.categories.main.model

        if not isinstance(model_name, str):
            return False

        if model_name.startswith('KDE'):
            return True

        raise ValueError(f'Invalid PDF found: {model_name}')
    # ------------------------
    def _get_kde(self) -> zpdf|None:
        '''
        - Makes KDE PDF
        - Saves fit (plot, list of parameters, etc)

        Returns
        ------------------
        - KDE PDF after fit
        - None if there are fewer than _min_kde_entries
        '''
        model_name  = self._cfg.categories.main.model
        data        = self._d_data['main']

        kde_builder = getattr(zfit.pdf, model_name)
        if data.nevents < self._min_kde_entries:
            pdf = None
        else:
            if 'options' in self._cfg.fit:
                cfg    = self._cfg.fit.get('options')
                kwargs = OmegaConf.to_container(cfg, resolve=True)
                kwargs = cast(dict, kwargs)
            else:
                kwargs = {}

            log.debug('KDE options:')
            log.debug(yaml.dump(kwargs))

            pdf = kde_builder(obs=self._obs, data=data, name=self._component, **kwargs)

        self._save_fit(
            cut_cfg  = self._get_cut_config(),
            plt_cfg  = self._cfg.plots,
            data     = data,
            model    = pdf,
            res      = None,
            out_path = f'{self._out_path}/main')

        return pdf
    # ------------------------
    def _get_ccbar_component(self) -> zpdf|None:
        '''
        This is an interace to the PRec class, which is in
        charge of building the KDE for ccbar sample

        Returns
        ----------------
        Either:

        - PDF with KDEs added corresponding to different groups of ccbar decays
        - None, if no data was found
        '''
        ftr=PRec(
            trig    = self._trigger,
            q2bin   = self._q2bin  ,
            d_weight= self._cfg.weights,
            out_dir = self._base_path,
            samples = self._cfg.ccbar_samples)

        obs_name = sut.name_from_obs(self._obs)

        kwargs   = OmegaConf.to_container(self._cfg.fitting, resolve=True)
        kwargs   = cast(dict, kwargs)

        pdf =ftr.get_sum(
            mass   = obs_name,
            name   = r'$c\bar{c}+X$',
            obs    = self._obs,
            **kwargs)

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
        if 'ccbar_samples' in self._cfg:
            return self._get_ccbar_component()

        if 'sample' not in self._cfg:
            return self._get_nomc_component()

        result_path = f'{self._out_path}/parameters.yaml'
        if self._copy_from_cache():
            res      = OmegaConf.load(result_path)
            res      = cast(DictConfig, res)
            # If caching, need only model, second return value
            # Is an empty DictConfig, because no fit happened
            val      = self._get_full_model(skip_fit=True)
            if val is None:
                return None

            model, _ = val 
            model    = self._fix_tails(pdf=model, pars=res)

            return model

        log.info(f'Fitting, could not find cached parameters in {result_path}')

        if self._is_kde():
            return self._get_kde()

        val = self._get_full_model(skip_fit=False)
        if val is None:
            return None

        full_model, cres = val

        OmegaConf.save(cres, result_path)

        self._cache()
        return full_model
# ------------------------
