'''
Module with utility functions related to the dmu.stats project
'''
# pylint: disable=import-error

import os
import re
import pickle
from typing import Union

import numpy
import pandas            as pnd
import matplotlib.pyplot as plt

import dmu.pdataframe.utilities as put
import dmu.generic.utilities    as gut

from dmu.stats.zfit         import zfit
from dmu.stats.fitter       import Fitter
from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.logging.log_store  import LogStore

import tensorflow as tf

from omegaconf              import OmegaConf, DictConfig
from zfit.core.interfaces   import ZfitData   as zdata
from zfit.core.interfaces   import ZfitSpace  as zobs
from zfit.core.interfaces   import ZfitPDF    as zpdf
from zfit.core.parameter    import Parameter  as zpar
from zfit.result            import FitResult  as zres

log = LogStore.add_logger('dmu:stats:utilities')
#-------------------------------------------------------
class Data:
    '''
    Data class
    '''
    weight_name = 'weight'
#-------------------------------------------------------
def name_from_obs(obs : zobs) -> str:
    '''
    Takes zfit observable, returns its name
    It is assumed this is a 1D observable
    '''
    if not isinstance(obs.obs, tuple):
        raise ValueError(f'Cannot retrieve name for: {obs}')

    if len(obs.obs) != 1:
        raise ValueError(f'Observable is not 1D: {obs.obs}')

    return obs.obs[0]
#-------------------------------------------------------
def range_from_obs(obs : zobs) -> tuple[float,float]:
    '''
    Takes zfit observable, returns tuple with two floats, representing range
    '''
    if not isinstance(obs.limits, tuple):
        raise ValueError(f'Cannot retrieve name for: {obs}')

    if len(obs.limits) != 2:
        raise ValueError(f'Observable has more than one range: {obs.limits}')

    minx, maxx = obs.limits

    return float(minx[0][0]), float(maxx[0][0])
#-------------------------------------------------------
def yield_from_zdata(data : zdata) -> float:
    '''
    Parameter
    --------------
    data : Zfit dataset

    Returns
    --------------
    Yield, i.e. number of entries or sum of weights if weighted dataset
    '''

    if data.weights is not None:
        val     = data.weights.numpy().sum()
    else:
        arr_val = data.to_numpy()
        val     = len(arr_val)

    if val < 0:
        raise ValueError(f'Yield cannot be negative, found {val}')

    return val
#-------------------------------------------------------
# Check PDF
#-------------------------------------------------------
def is_pdf_usable(pdf : zpdf) -> zpdf:
    '''
    Will check if the PDF is usable
    '''
    [[[minx]], [[maxx]]]= pdf.space.limits

    arr_x = numpy.linspace(minx, maxx, 100)

    try:
        pdf.pdf(arr_x)
    except tf.errors.InvalidArgumentError:
        log.warning('PDF cannot be evaluated')
        return False

    return True
#-------------------------------------------------------
#Zfit/print_pdf
#-------------------------------------------------------
def _get_const(par : zpar , d_const : Union[None, dict[str, tuple[float,float]]]) -> str:
    '''
    Takes zfit parameter and dictionary of constraints
    Returns a formatted string with the value of the constraint on that parameter
    '''
    if d_const is None or par.name not in d_const:
        return 'none'

    obj = d_const[par.name]
    if isinstance(obj, (list, tuple)):
        [mu, sg] = obj
        val      = f'{mu:.3e}___{sg:.3e}' # This separator needs to be readable and not a space
    else:
        val      = str(obj)

    return val
#-------------------------------------------------------
def _blind_vars(s_par : set, l_blind : Union[list[str], None] = None) -> set[zpar]:
    '''
    Takes set of zfit parameters and list of parameter names to blind
    returns set of zfit parameters that should be blinded
    '''
    if l_blind is None:
        return s_par

    rgx_ors = '|'.join(l_blind)
    regex   = f'({rgx_ors})'

    s_par_blind = { par for par in s_par if not re.match(regex, par.name) }

    return s_par_blind
#-------------------------------------------------------
def _get_pars(
        pdf   : zpdf,
        blind : Union[None, list[str]]) -> tuple[list, list]:

    s_par_flt = pdf.get_params(floating= True)
    s_par_fix = pdf.get_params(floating=False)

    s_par_flt = _blind_vars(s_par_flt, l_blind=blind)
    s_par_fix = _blind_vars(s_par_fix, l_blind=blind)

    l_par_flt = list(s_par_flt)
    l_par_fix = list(s_par_fix)

    l_par_flt = sorted(l_par_flt, key=lambda par: par.name)
    l_par_fix = sorted(l_par_fix, key=lambda par: par.name)

    return l_par_flt, l_par_fix
#-------------------------------------------------------
def _get_messages(
        pdf       : zpdf,
        l_par_flt : list,
        l_par_fix : list,
        d_const   : Union[None, dict[str,list[float]]] = None) -> list[str]:

    str_space = str(pdf.space)

    l_msg=[]
    l_msg.append('-' * 20)
    l_msg.append(f'PDF: {pdf.name}')
    l_msg.append(f'OBS: {str_space}')
    l_msg.append(f'{"Name":<50}{"Value":>15}{"Low":>15}{"High":>15}{"Floating":>5}{"Constraint":>25}')
    l_msg.append('-' * 20)
    for par in l_par_flt:
        value = par.value().numpy()
        low   = par.lower
        hig   = par.upper
        const = _get_const(par, d_const)
        l_msg.append(f'{par.name:<50}{value:>15.3e}{low:>15.3e}{hig:>15.3e}{par.floating:>5}{const:>25}')

    l_msg.append('')

    for par in l_par_fix:
        value = par.value().numpy()
        low   = par.lower
        hig   = par.upper
        const = _get_const(par, d_const)
        l_msg.append(f'{par.name:<50}{value:>15.3e}{low:>15.3e}{hig:>15.3e}{par.floating:>5}{const:>25}')

    return l_msg
#-------------------------------------------------------
def print_pdf(
        pdf      : zpdf,
        d_const  : Union[None, dict[str,tuple[float, float]]] = None,
        txt_path : Union[str,None]                            = None,
        level    : int                                        = 20,
        blind    : Union[None, list[str]]                     = None):
    '''
    Function used to print zfit PDFs

    Parameters
    -------------------
    pdf (zfit.PDF): PDF
    d_const (dict): Optional dictionary mapping {par_name : [mu, sg]}
    txt_path (str): Optionally, dump output to text in this path
    level (str)   : Optionally set the level at which the printing happens in screen, default info
    blind (list)  : List of regular expressions matching variable names to blind in printout
    '''
    l_par_flt, l_par_fix = _get_pars(pdf, blind)
    l_msg                = _get_messages(pdf, l_par_flt, l_par_fix, d_const)

    if txt_path is not None:
        log.debug(f'Saving to: {txt_path}')
        message  = '\n'.join(l_msg)
        dir_path = os.path.dirname(txt_path)
        os.makedirs(dir_path, exist_ok=True)
        with open(txt_path, 'w', encoding='utf-8') as ofile:
            ofile.write(message)

        return

    for msg in l_msg:
        if   level == 20:
            log.info(msg)
        elif level == 30:
            log.debug(msg)
        else:
            raise ValueError(f'Invalid level: {level}')
#---------------------------------------------
def _parameters_from_result(result : zres) -> dict[str,tuple[float,float]]:
    d_par = {}
    log.debug('Reading parameters from:')
    if log.getEffectiveLevel() == 10:
        print(result)

    log.debug(60 * '-')
    log.debug('Reading parameters')
    log.debug(60 * '-')
    for name, d_val in result.params.items():
        value = d_val['value']
        error = None
        if 'hesse'         in d_val:
            error = d_val['hesse']['error']

        if 'minuit_hesse'  in d_val:
            error = d_val['minuit_hesse']['error']

        log.debug(f'{name:<20}{value:<20.3f}{error}')

        d_par[name] = value, error

    return d_par
#---------------------------------------------
def save_fit(
        data    : zdata,
        model   : zpdf|None,
        res     : zres|None,
        fit_dir : str,
        d_const : dict[str,tuple[float,float]]|None = None) -> None:
    '''
    Function used to save fit results, meant to reduce boiler plate code

    Plots: If:

    ptr = ZFitPlotter(data=dat, model=pdf)
    ptr.plot()

    was done before calling this method, the plot will also be saved

    Parameters
    --------------------
    model: PDF to be plotted, if None, will skip steps
    '''
    os.makedirs(fit_dir, exist_ok=True)
    log.info(f'Saving fit to: {fit_dir}')

    if plt.get_fignums():
        fit_path = f'{fit_dir}/fit.png'
        log.info(f'Saving fit to: {fit_path}')
        plt.savefig(fit_path)
        plt.close('all')
    else:
        log.info('No fit plot found')

    _save_result(fit_dir=fit_dir, res=res)

    df     = data.to_pandas(weightsname=Data.weight_name)
    opath  = f'{fit_dir}/data.json'
    log.debug(f'Saving data to: {opath}')
    df.to_json(opath, indent=2)

    if model is None:
        return

    print_pdf(model, txt_path=f'{fit_dir}/post_fit.txt', d_const=d_const)
    pdf_to_tex(path=f'{fit_dir}/post_fit.txt', d_par={'mu' : r'$\mu$'}, skip_fixed=True)
#-------------------------------------------------------
def _save_result(fit_dir : str, res : zres|None) -> None:
    '''
    Saves result as yaml, JSON, pkl

    Parameters
    ---------------
    fit_dir: Directory where fit result will go
    res    : Zfit result object
    '''
    if res is None:
        log.info('No result object found, not saving parameters in pkl or JSON')
        return

    # TODO: Remove this once there be a safer way to freeze
    # see https://github.com/zfit/zfit/issues/632
    try:
        res.freeze()
    except AttributeError:
        pass

    with open(f'{fit_dir}/fit.pkl', 'wb') as ofile:
        pickle.dump(res, ofile)

    d_par  = _parameters_from_result(result=res)
    opath  = f'{fit_dir}/parameters.json'
    log.debug(f'Saving parameters to: {opath}')
    gut.dump_json(d_par, opath)

    opath  = f'{fit_dir}/parameters.yaml'
    cres   = zres_to_cres(res=res)
    OmegaConf.save(cres, opath)
#-------------------------------------------------------
# Make latex table from text file
#-------------------------------------------------------
def _reformat_expo(val : str) -> str:
    regex = r'([\d\.]+)e([-,\d]+)'
    mtch  = re.match(regex, val)
    if not mtch:
        raise ValueError(f'Cannot extract value and exponent from: {val}')

    [val, exp] = mtch.groups()
    exp        = int(exp)

    return f'{val}\cdot 10^{{{exp}}}'
#-------------------------------------------------------
def _format_float_str(val : str) -> str:
    '''
    Takes number as string and returns a formatted version
    '''

    fval = float(val)
    if abs(fval) > 1000:
        return f'{fval:,.0f}'

    val = f'{fval:.3g}'
    if 'e' in val:
        val = _reformat_expo(val)

    return val
#-------------------------------------------------------
def _info_from_line(line : str) -> tuple|None:
    regex = r'(^\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)'
    mtch  = re.match(regex, line)
    if not mtch:
        return None

    log.debug(f'Reading information from: {line}')

    [par, _, low, high, floating, cons] = mtch.groups()

    low  = _format_float_str(low)
    high = _format_float_str(high)

    if cons != 'none':
        [mu, sg] = cons.split('___')

        mu   = _format_float_str(mu)
        sg   = _format_float_str(sg)

        cons = f'$\mu={mu}; \sigma={sg}$'

    return par, low, high, floating, cons
#-------------------------------------------------------
def _df_from_lines(l_line : list[str]) -> pnd.DataFrame:
    df = pnd.DataFrame(columns=['Parameter', 'Low', 'High', 'Floating', 'Constraint'])

    for line in l_line:
        info = _info_from_line(line=line)
        if info is None:
            continue

        par, low, high, floating, cons = info

        df.loc[len(df)] = {'Parameter' : par,
                           'Low'       : low,
                           'High'      : high,
                           'Floating'  : floating,
                           'Constraint': cons,
                           }

    return df
#-------------------------------------------------------
def pdf_to_tex(path : str, d_par : dict[str,str], skip_fixed : bool = True) -> None:
    '''
    Takes

    path: path to a `txt` file produced by stats/utilities:print_pdf
    d_par: Dictionary mapping parameter names in this file to proper latex names

    Creates a latex table with the same name as `path` but `txt` extension replaced by `tex`
    '''

    path = str(path)
    with open(path, encoding='utf-8') as ifile:
        l_line = ifile.read().splitlines()
        l_line = l_line[4:] # Remove header

    df = _df_from_lines(l_line)
    df['Parameter']=df.Parameter.apply(lambda x : d_par.get(x, x.replace('_', ' ')))

    out_path = path.replace('.txt', '.tex')

    if skip_fixed:
        df = df[df.Floating == '1']
        df = df.drop(columns='Floating')

    df_1 = df[df.Constraint == 'none']
    df_2 = df[df.Constraint != 'none']

    df_1 = df_1.sort_values(by='Parameter', ascending=True)
    df_2 = df_2.sort_values(by='Parameter', ascending=True)
    df   = pnd.concat([df_1, df_2])

    put.df_to_tex(df, out_path)
#---------------------------------------------
# Fake/Placeholder fit
#---------------------------------------------
def get_model(
        kind : str,
        obs  : zobs|None = None,
        lam  : float     = -0.0001) -> zpdf:
    '''
    Returns zfit PDF for tests

    Parameters:

    kind: 'signal' for Gaussian, 's+b' for Gaussian plus exponential
    obs : If provided, will use it, by default None and will be built in function
    lam : Decay constant of exponential component, set to -0.0001 by default
    '''
    if obs is None:
        obs  = zfit.Space('mass', limits=(4500, 7000))

    mu   = zfit.Parameter('mu', 5200, 4500, 6000)
    sg   = zfit.Parameter('sg',   50,   10, 200)
    gaus = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)

    if kind == 'signal':
        return gaus

    c   = zfit.Parameter('c', lam, -0.01, 0)
    expo= zfit.pdf.Exponential(obs=obs, lam=c)

    if kind == 's+b':
        nexpo = zfit.param.Parameter('nbkg', 1000, 0, 1000_000)
        ngaus = zfit.param.Parameter('nsig', 1000, 0, 1000_000)

        bkg   = expo.create_extended(nexpo)
        sig   = gaus.create_extended(ngaus)
        pdf   = zfit.pdf.SumPDF([bkg, sig])

        return pdf

    raise NotImplementedError(f'Invalid kind of fit: {kind}')
#---------------------------------------------
def _pdf_to_data(pdf : zpdf, add_weights : bool) -> zdata:
    nentries = 10_000
    data     = pdf.create_sampler(n=nentries)
    if not add_weights:
        return data

    arr_wgt  = numpy.random.normal(loc=1, scale=0.1, size=nentries)
    data     = data.with_weights(arr_wgt)

    return data
#---------------------------------------------
def placeholder_fit(
        kind     : str,
        fit_dir  : str,
        df       : pnd.DataFrame|None = None,
        plot_fit : bool               = True) -> None:
    '''
    Function meant to run toy fits that produce output needed as an input
    to develop tools on top of them

    kind: Kind of fit, e.g. s+b for the simples signal plus background fit
    fit_dir: Directory where the output of the fit will go
    df: pandas dataframe if passed, will reuse that data, needed to test data caching
    plot_fit: Will plot the fit or not, by default True
    '''
    pdf  = get_model(kind)
    print_pdf(pdf, txt_path=f'{fit_dir}/pre_fit.txt')
    if df is None:
        log.warning('Using user provided data')
        data = _pdf_to_data(pdf=pdf, add_weights=True)
    else:
        data = zfit.Data.from_pandas(df, obs=pdf.space, weights=Data.weight_name)

    d_const = {'sg' : [50, 3]}

    obj = Fitter(pdf, data)
    res = obj.fit(cfg={'constraints' : d_const})

    if plot_fit:
        obj   = ZFitPlotter(data=data, model=pdf)
        obj.plot(nbins=50, stacked=True)

    save_fit(data=data, model=pdf, res=res, fit_dir=fit_dir, d_const=d_const)
#---------------------------------------------
def _reformat_values(d_par : dict) -> dict:
    '''
    Parameters
    --------------
    d_par: Dictionary formatted as:

        {'minuit_hesse': {'cl': 0.6,
                         'error': np.float64(0.04),
                         'weightcorr': <WeightCorr.FALSE: False>},
         'value'       : 0.34},

    Returns
    --------------
    Dictionary formatted as:

    {
        'error' : 0.04,
        'value' : 0.34
    }
    '''

    error = d_par['minuit_hesse']['error']
    error = float(error)

    value = d_par['value']

    return {'value' : value, 'error' : error}
#---------------------------------------------
def zres_to_cres(res : zres) -> DictConfig:
    '''
    Parameters
    --------------
    res : Zfit result object

    Returns
    --------------
    OmegaConfig's DictConfig instance
    '''
    # This should prevent crash when result object was already frozen
    try:
        res.freeze()
    except AttributeError:
        pass

    par   = res.params
    d_par = { name : _reformat_values(d_par=d_par) for name, d_par in par.items()}
    cfg   = OmegaConf.create(d_par)

    return cfg
#---------------------------------------------
