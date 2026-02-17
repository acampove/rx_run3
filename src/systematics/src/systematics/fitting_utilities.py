'''
Module with functions needed for fitting
'''
import zfit
import numpy

from typing        import Callable, Literal, overload
from functools     import lru_cache
from rpk_log_store import log_store  as LogStore

from rpk_tools     import FitConfig
from rpk_tools     import PkeeBuilder    , PkmmBuilder
from rpk_tools     import PkJpsiEEBuilder, PkJpsiMMBuilder 
from rpk_tools     import RpKPDF
from rpk_tools     import mc_plotter
from rpk_tools     import dt_ee_rare as dt_ee_rare_plotter
from rpk_tools     import dt_ee_reso as dt_ee_reso_plotter
from rpk_tools     import dt_mm_rare as dt_mm_rare_plotter
from rpk_tools     import dt_mm_reso as dt_mm_reso_plotter
from rpk_tools     import save_json

from .types        import Component
from .holder       import Holder

zpar = zfit.param.Parameter

ChanelLit= Literal['ee', 'mm']
Builder  = PkmmBuilder | PkeeBuilder | PkJpsiMMBuilder | PkJpsiEEBuilder

log = LogStore.add_logger('rpk_tools::fitting_utilities')
# ----------------------
@overload
def _get_builder(
    cfg    : FitConfig, 
    is_rare: Literal[False],
    channel: Literal['mm'],
    params : tuple[tuple[str,zpar],...]) -> PkJpsiMMBuilder:...
@overload
def _get_builder(
    cfg    : FitConfig, 
    is_rare: Literal[False],
    channel: Literal['ee'],
    params : tuple[tuple[str,zpar],...]) -> PkJpsiEEBuilder:...
# ----------------------
@overload
def _get_builder(
    cfg    : FitConfig, 
    is_rare: Literal[True],
    channel: Literal['mm'],
    params : tuple[tuple[str,zpar],...]) -> PkmmBuilder:...
@overload
def _get_builder(
    cfg    : FitConfig, 
    is_rare: Literal[True],
    channel: Literal['ee'],
    params : tuple[tuple[str,zpar],...]) -> PkeeBuilder:...
# ----------------------
@lru_cache(maxsize = 10)
def _get_builder(
    cfg    : FitConfig, 
    is_rare: bool,
    channel: ChanelLit,
    params : tuple[tuple[str,zpar],...]) -> Builder:
    '''
    Parameters
    -------------
    cfg    : Fitting configuration
    is_rare: Rare or resonant model builder
    params : Tuple with pairs of name and parameter. Needed to do reparametrization for e.g. rpk
            Need tuple to allow hashing, to allow caching, to prevent excessive memory use

    Returns
    -------------
    Builder object
    '''
    if is_rare:
        Builder = PkmmBuilder     if channel == 'mm' else PkeeBuilder
    else:
        Builder = PkJpsiMMBuilder if channel == 'mm' else PkJpsiEEBuilder 

    log.info(f'No builder found, creating new one for channel {channel}')
    obs      = zfit.Space(cfg.obs_name, cfg.fit_range) 
    builder  = Builder(
        name   = channel, 
        params = dict(params),
        obs    = obs)

    return builder
# ----------------------
# Bulders 
# ----------------------
def build_models(
    cfg    : FitConfig, 
    params : dict[str,zpar],
    dat    : Holder[numpy.ndarray]) -> tuple[Holder[RpKPDF], Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    cfg: Fit configuration
    dat: Holder of datasets

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''
    builder = None

    if cfg.channel == 'ee' and cfg.is_rare:
        builder = _build_ee_rare_models

    if cfg.channel == 'mm' and cfg.is_rare:
        builder = _build_mm_rare_models

    if cfg.channel == 'ee' and not cfg.is_rare:
        builder = _build_ee_reso_models

    if cfg.channel == 'mm' and not cfg.is_rare:
        builder = _build_mm_reso_models

    if not isinstance(builder, Callable):
        raise ValueError(f'Invalid Channel: {cfg.channel}')

    return builder(cfg = cfg, params = params, dat = dat)
# ----------------------
# ----------------------
def _build_mm_reso_models(
    cfg   : FitConfig, 
    params: dict[str,zpar],
    dat   : Holder[numpy.ndarray]) -> tuple[Holder[RpKPDF],Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    cfg    : Fit configuration
    params : Dictionary of parameters used to carry out reparametrizations, e.g. with rpk
    dat    : Holder of datasets

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''
    builder    = _get_builder(
        cfg    = cfg, 
        channel= 'mm',
        is_rare= False,
        params = tuple(params.items()))

    models                    = Holder[RpKPDF]()
    models[Component.jpsi_mm] = builder.build_sig_model(kind = cfg.signal_model)
    models[Component.jpsi_mm].fit_to(
        data          = dat[Component.jpsi_mm], 
        ntries        = 3, 
        ranges        = [cfg.fit_range], 
        nbins_chi2test= cfg.plot_bin)
    
    builder.build_bkg_model(kind = cfg.comb_model)

    models, dat = _build_non_parametric_reso(
        builder = builder,
        models  = models, 
        dat     = dat,
        cfg     = cfg)

    models[Component.all] = builder.build_total_model()

    return models, dat
# ----------------------
def _build_ee_reso_models(
    dat    : Holder[numpy.ndarray],
    params : dict[str,zpar],
    cfg    : FitConfig) -> tuple[Holder[RpKPDF],Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    datasets: Container of dataframes    
    params  : Dictionary of parameters used to carry out reparametrizations, e.g. with rpk
    cfg     : Holder of configuration

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''
    builder    = _get_builder(
        cfg    = cfg, 
        is_rare= False,
        channel= 'ee',
        params = tuple(params.items()))

    models = Holder[RpKPDF]()
    models[Component.jpsi_ee] = builder.build_sig_model(kind = cfg.signal_model)
    models[Component.jpsi_ee].fit_to(
        data          = dat[Component.jpsi_ee],
        ntries        = 10,
        ranges        =[cfg.fit_range],
        nbins_chi2test= cfg.plot_bin)

    builder.build_bkg_model(kind = cfg.comb_model)

    models, datasets = _build_non_parametric_reso(
        builder = builder,
        models  = models, 
        dat     = dat,
        cfg     = cfg)

    models[Component.all] = builder.build_total_model()

    return models, datasets
# ----------------------
# ----------------------
def _build_mm_rare_models(
    cfg   : FitConfig, 
    params: dict[str,zpar],
    dat   : Holder[numpy.ndarray]) -> tuple[Holder[RpKPDF],Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    cfg    : Fit configuration
    params : Dictionary of parameters used to carry out reparametrizations, e.g. with rpk
    dat    : Holder of datasets

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''
    builder    = _get_builder(
        cfg    = cfg, 
        channel= 'mm',
        is_rare= True,
        params = tuple(params.items()))

    holder                 = Holder[RpKPDF]()
    holder[Component.pKmm] = builder.build_sig_model(kind = cfg.signal_model)
    holder[Component.pKmm].fit_to(
        data          = dat[Component.pKmm], 
        ntries        = 3, 
        ranges        = [cfg.fit_range], 
        nbins_chi2test= cfg.plot_bin)
    
    builder.build_bkg_model(kind = cfg.comb_model)

    holder[Component.all] = builder.build_total_model()

    return holder, dat
# ----------------------
def _build_ee_rare_models(
    dat    : Holder[numpy.ndarray],
    params : dict[str,zpar],
    cfg    : FitConfig) -> tuple[Holder[RpKPDF],Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    datasets: Container of dataframes    
    params  : Dictionary of parameters used to carry out reparametrizations, e.g. with rpk
    cfg     : Holder of configuration

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''
    if cfg.channel != 'ee':
        raise ValueError(f'Invalid channel: {cfg.channel}')

    builder    = _get_builder(
        cfg    = cfg, 
        is_rare= True,
        channel= cfg.channel,
        params = tuple(params.items()))

    models        = Holder[RpKPDF]()
    models[Component.pKee]   = builder.build_sig_model(kind = cfg.signal_model)
    models[Component.pKee].fit_to(
        data          = dat[Component.pKee],
        ntries        = 10,
        ranges        =[cfg.fit_range],
        nbins_chi2test= cfg.plot_bin)

    models[Component.comb] = builder.build_bkg_model(kind = cfg.comb_model)
    models[Component.comb].fit_to(
        data          = dat[Component.comb],
        ntries        = 5,
        ranges        = [cfg.fit_range],
        nbins_chi2test= cfg.plot_bin)

    models, datasets = _build_non_parametric_rare(
        builder = builder,
        models  = models, 
        dat     = dat,
        cfg     = cfg)

    trig = cfg.trig
    if   trig in ['L0E']:
        sigma_ratio = 1.05
    elif trig in ['L0I', 'L0I2']:
        sigma_ratio = 1.06
    else:
        raise ValueError(f'Invalid trigger: {trig}')

    models[Component.all] = builder.build_total_model(sigma_ratio = sigma_ratio)

    return models, datasets
# ----------------------
# ----------------------
def _build_non_parametric_rare(
    builder: PkeeBuilder,
    models : Holder[RpKPDF], 
    dat    : Holder[numpy.ndarray],
    cfg    : FitConfig) -> tuple[Holder[RpKPDF], Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    builder: Builder of PDFs
    models : Holder of models, needed to pick new models
    dat    : Data used to build KDEs
    cfg    : Holder of configs

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''

    nevt = {
        Component.pKKK   : 5,
        Component.pKKpi  : 5,
        Component.pKpipi : 5}

    if isinstance(cfg, FitConfig) and cfg.systematics.mutable_cfg['run_npv']:
        log.info('Using NPV systematics')

        npv  = cfg.systematics.npv
        nevt = npv.recalculate_yields(yields = nevt)
        dat  = npv.bootstrap(holder = dat)
    else:
        log.info('Not using NPV systematics')

    builder.set_hadron_misID_yields(yields = nevt)
    for comp, array in dat.items():
        if comp in models:
            continue

        if comp == Component.all:
            continue

        log.debug(f'Building KDE for component: {comp}')
        models[comp] = builder.build_KDE_model(
            array = array, 
            comp  = comp)

    return models, dat
# ----------------------
def _build_non_parametric_reso(
    builder: PkJpsiEEBuilder | PkJpsiMMBuilder,
    models : Holder[RpKPDF], 
    dat    : Holder[numpy.ndarray],
    cfg    : FitConfig) -> tuple[Holder[RpKPDF], Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    builder: Builder of PDFs
    models : Holder of models, needed to pick new models
    dat    : Data used to build KDEs
    cfg    : Holder of configs

    Returns
    -------------
    Tuple with:

    - Holder of fitted PDFs
    - Datasets used for fit. Different from inputs if bootstrapping was used
    '''
    if isinstance(cfg, FitConfig) and cfg.systematics.mutable_cfg['run_npv']:
        log.info('Using NPV systematics')

        npv  = cfg.systematics.npv
        dat  = npv.bootstrap(holder = dat)
    else:
        log.info('Not using NPV systematics')

    for comp, array in dat.items():
        if comp in models:
            continue

        if comp == Component.all:
            continue

        log.debug(f'Building KDE for component: {comp}')
        models[comp] = builder.build_KDE_model(
            array    = array, 
            comp     = comp)

    return models, dat
# ----------------------
# Savers
# ----------------------
def save_outputs(
    dat     : Holder[numpy.ndarray],
    pdf     : Holder[RpKPDF],
    cfg     : FitConfig,
    kind    : str,
    only_mc : bool = False) -> None:
    '''
    Parameters
    ---------------
    datasets: Holder of the data used for the fit, real and simulated
    model   : Holder of models used in fit
    cfg     : Object storing configuration
    kind    : Type of output, e.g. nominal, alt_sig, etc
    only_mc : If True, will only save plots for MC fit components
    '''
    if   cfg.channel == 'ee':
        fun = _save_ee_outputs
    elif cfg.channel == 'mm':
        fun = _save_mm_outputs
    else:
        raise ValueError(f'Invalid channel: {cfg.channel}')

    fun(
        cfg     = cfg, 
        dat     = dat, 
        pdf     = pdf,
        kind    = kind,
        only_mc = only_mc)
# ----------------------
def _save_mm_outputs(
    dat     : Holder[numpy.ndarray],
    pdf     : Holder[RpKPDF],
    cfg     : FitConfig,
    kind    : str,
    only_mc : bool = False) -> None:
    '''
    Parameters
    ---------------
    datasets: Holder of the data used for the fit, real and simulated
    model   : Holder of models used in fit
    cfg     : Object storing configuration
    kind    : Type of output, e.g. nominal, alt_sig, etc
    only_mc : If True, will only save plots for MC fit components
    '''
    mc_dir = cfg.plot_dir / f'{kind}/mc_shapes'
    for comp in dat.keys:
        if comp == Component.all:
            continue

        mc_plotter(
            array     = dat[comp],
            model     = pdf[comp], 
            plot_range= cfg.fit_range,
            plot_dir  = mc_dir,
            component = comp,
            nbins     = cfg.plot_bin)

    if only_mc:
        return

    if   cfg.is_rare:
        dt_plotter = dt_mm_rare_plotter
    else:
        dt_plotter = dt_mm_reso_plotter

    dt_dir = cfg.plot_dir / f'{kind}/dt_shapes'
    dt_plotter(
        array     = dat[Component.all], 
        model     = pdf[Component.all], 
        plot_range= cfg.fit_range, 
        plot_dir  = mc_dir, 
        plot_name = 'data_fit', 
        text      = cfg.trig, 
        stacked   = False, 
        nbins     = cfg.plot_bin)

    dt_plotter(
        array     = dat[Component.all], 
        model     = pdf[Component.all], 
        plot_range= cfg.fit_range,
        plot_dir  = dt_dir,
        plot_name = 'data_fit',
        text      = cfg.trig,
        stacked   = True,
        nbins     = cfg.plot_bin)

    for comp in pdf.keys:
        save_json(pdf[comp], json_dir=cfg.json_dir, json_name='mc_fit')
        save_json(pdf[comp], json_dir=cfg.json_dir, json_name='data_fit')
# ----------------------
def _save_ee_outputs(
    dat     : Holder[numpy.ndarray],
    pdf     : Holder[RpKPDF],
    cfg     : FitConfig,
    kind    : str,
    only_mc : bool = False) -> None:
    '''
    Parameters
    ---------------
    datasets: Holder of the data used for the fit, real and simulated
    model   : Holder of models used in fit
    cfg     : Object storing configuration
    kind    : Type of output, e.g. nominal, alt_sig, etc
    only_mc : If True, will only save plots for MC fit components
    '''

    mc_dir = cfg.plot_dir / f'{kind}/mc_shapes'
    for comp in dat.keys:
        if comp == Component.all:
            continue

        model = pdf[comp]
        data  = dat[comp]
        mc_plotter(
            array     = data, 
            model     = model, 
            plot_range= cfg.fit_range, 
            plot_dir  = mc_dir, 
            component = comp, 
            nbins     = cfg.plot_bin)

    if only_mc:
        return

    if cfg.is_rare:
        dt_plotter = dt_ee_rare_plotter
    else:
        dt_plotter = dt_ee_reso_plotter

    dt_dir = cfg.plot_dir / f'{kind}/dt_shapes'

    all = Component.all
    dt_plotter(dat[all], pdf[all], plot_range=cfg.fit_range, plot_dir=dt_dir, plot_name='data_fit', text=cfg.trig, stacked=False, nbins=cfg.plot_bin)
    dt_plotter(dat[all], pdf[all], plot_range=cfg.fit_range, plot_dir=dt_dir, plot_name='data_fit', text=cfg.trig, stacked=True , nbins=cfg.plot_bin)

    js_dir = cfg.json_dir / kind

    for comp in pdf.keys: 
        model = pdf[comp]

        if model.is_kde:
            continue

        save_json(model, json_dir=js_dir, json_name=comp)
# ----------------------
