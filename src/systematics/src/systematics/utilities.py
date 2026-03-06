'''
Module with functions needed to calculate systematics
'''
import numpy

from typing        import Literal
from dmu           import LogStore
from dmu.stats     import zfit
from dmu.stats     import FitResult
from dmu.generic   import utilities as gut
from fitter        import ChannelHolder
from fitter        import ComponentHolder
from fitter        import FitConfig
from .systematics  import Calculator as SystematicsCalculator

log     = LogStore.add_logger('systematics:utilities')
zres    = zfit.result.FitResult
zlos    = zfit.loss.ExtendedUnbinnedNLL
zpar    = zfit.param.Parameter
Channel = Literal['ee', 'mm']

# Holders of objects
hdat    = ComponentHolder[numpy.ndarray]

# Holders of objects by channel
cfcg    = ChannelHolder[FitConfig]
# ----------------------
def calculate_sim_systematics(
    nll          : ChannelHolder[zlos],
    dat          : ChannelHolder[hdat],
    cfg          : cfcg,
    params       : dict[str,zpar],
    res          : FitResult,
    only_nominal : bool = False) -> None:
    '''
    Parameters
    -------------
    nll    : Likelihood after fit
    dat    : Object holding datasets
    cfg    : Object holding configuration needed for toys
    params : Parameters needed to reparametrize mode, with e.g. rpk
    res    : Result of nominal fit to real data
    only_nominal: If true, will only make toys for nominal model, False by default
    '''
    toys_cfg = gut.load_conf(
        package = 'rpk_configs', 
        fpath   = 'toys/generic.yaml')

    toys_cfg.rseed = cfg.ee.rseed
    poi            = cfg.ee.poi

    scal = SystematicsCalculator(
        obj = nll, 
        res = res,
        poi = poi,
        cfg = toys_cfg) 

    scal.switch_model(
        model = None,
        label = 'nominal')

    if only_nominal:
        return

    _run_model_systematics(
        scal   = scal,
        cfg    = cfg.ee,
        params = params,
        dat    = dat.ee)

    _run_model_systematics(
        scal   = scal,
        cfg    = cfg.mm,
        params = params,
        dat    = dat.mm)

    _run_model_assessment(scal = scal)
# ----------------------
def _run_model_assessment(scal : SystematicsCalculator) -> None:
    '''
    Parameters
    -------------
    scal   : Object that runs toy fits
    channel: Either ee or mm
    '''
    for name, par in scal.parameters.items():
        if 'dt_sig_N' in name:
            continue

        if name == scal.poi:
            continue

        label = f'fix_{name}'

        scal.fix_parameter(
            name  = name, 
            label = label)

    for par in scal.parameters.values():
        par.floating = True
# ----------------------
def _run_model_systematics(
    scal  : SystematicsCalculator,
    cfg   : FitConfig,
    params: dict[str,zpar],
    dat   : Holder[numpy.ndarray]) -> None:
    '''
    Parameters
    -------------
    nll    : Likelihood after fit
    dat    : Object holding datasets
    params : Parameters needed to reparametrize mode, with e.g. rpk
    cfg    : Object holding configuration needed for toys
    '''
    # -----------------------
    # Non parametric PDF systematics 
    # -----------------------
    cfg.systematics.mutable_cfg['run_npv'] = True
    while cfg.systematics.npvs:
        _run_systematic(
            name     = cfg.systematics.this_npv_name,
            scal     = scal,
            cfg      = cfg,
            params   = params,
            plot     = False, 
            datasets = dat)

    cfg.systematics.mutable_cfg['run_npv'] = False 
    # -----------------------
    # Signal model systematic
    # -----------------------
    cfg_sig, name = cfg.vary_model(kind = 'sig')
    _run_systematic(
        name     = name, 
        scal     = scal,
        cfg      = cfg_sig,
        params   = params,
        datasets = dat)
    # -----------------------
    # Combinatorial model systematic
    # -----------------------
    cfg_cmb, name = cfg.vary_model(kind = 'cmb')
    _run_systematic(
        name     = name,
        scal     = scal,
        cfg      = cfg_cmb,
        params   = params,
        datasets = dat)
# ----------------------
def _run_systematic(
    name    : str,
    scal    : SystematicsCalculator,
    cfg     : FitConfig, 
    params  : dict[str,zpar],
    datasets: Holder[numpy.ndarray],
    plot    : bool = True) -> None:
    '''
    Parameters
    -------------
    name    : Systematics label, e.g. alt_sig
    scal    : Systematics calculator
    cfg     : Holder of configurations to make model
    plot    : If true, it will save the plots from fits
    params  : Parameters needed to reparametrize mode, with e.g. rpk
    datasets: Container of datasets, needed to fit new model components
    '''
    log.info(f'Fitting for systematic: {name}')

    models, new_data= fut.build_models(
        dat    = datasets, 
        params = params, 
        cfg    = cfg)

    if not isinstance(models[Component.all], RpKSumPDF):
        raise ValueError('Full model is not a SumPDF')

    full_model = models[Component.all]
    if not isinstance(full_model, RpKSumPDF):
        raise ValueError('Full model is not an RpKSumPDF: {full_model}')

    scal.switch_model(
        model  = full_model,
        channel= cfg.channel,
        label  = name)

    if not plot:
        return

    fut.save_outputs(
        dat      = new_data,
        pdf      = models,
        kind     = name,
        cfg      = cfg,
        only_mc  = True) # For toys only save MC fit plots
# ----------------------
