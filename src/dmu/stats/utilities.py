'''
Module with utility functions related to the dmu.stats project
'''
import os
import re
import pickle
from typing import Union

import zfit
from zfit.core.data         import Data       as zdata
from zfit.core.basepdf      import BasePDF    as zpdf
from zfit.result            import FitResult  as zres

import pandas            as pnd
import matplotlib.pyplot as plt

import dmu.pdataframe.utilities as put

from dmu.stats.fitter       import Fitter
from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('dmu:stats:utilities')
#-------------------------------------------------------
#Zfit/print_pdf
#-------------------------------------------------------
def _get_const(par : zfit.Parameter, d_const : Union[None, dict[str, list[float]]]) -> str:
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
def _blind_vars(s_par : set, l_blind : Union[list[str], None] = None) -> set[zfit.Parameter]:
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
        pdf : zfit.pdf.BasePDF,
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
        pdf       : zfit.pdf.BasePDF,
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
        pdf      : zfit.pdf.BasePDF,
        d_const  : Union[None, dict[str,list[float]]] = None,
        txt_path : Union[str,None]                    = None,
        level    : int                                = 20,
        blind    : Union[None, list[str]]             = None):
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
def save_fit(
        data    : zdata,
        model   : zpdf,
        res     : zres,
        fit_dir : str,
        d_const : dict[str,list[str]] = None) -> None:
    '''
    Function used to save fit results, meant to reduce boiler plate code
    '''
    os.makedirs(fit_dir, exist_ok=True)

    res.freeze()
    with open(f'{fit_dir}/fit.pkl', 'wb') as ofile:
        pickle.dump(res, ofile)

    plt.savefig(f'{fit_dir}/fit.png')

    print_pdf(model, txt_path=f'{fit_dir}/post_fit.txt', d_const=d_const)
    pdf_to_tex(path=f'{fit_dir}/post_fit.txt', d_par={'mu' : r'$\mu$'}, skip_fixed=True)
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
    val = float(val)

    if abs(val) > 1000:
        return f'{val:,.0f}'

    val = f'{val:.3g}'

    if 'e' in val:
        val = _reformat_expo(val)

    return val
#-------------------------------------------------------
def _info_from_line(line : str) -> [tuple,None]:
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
def _get_model(kind : str):
    obs   = zfit.Space('mass', limits=(0, 10))
    mu    = zfit.Parameter('mu', 5.0, -1, 5)
    sg    = zfit.Parameter('sg', 0.5,  0, 5)
    gauss = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)

    if kind == 'signal':
        return gauss

    c     = zfit.Parameter('c', -0.1, -1, 0)
    expo  = zfit.pdf.Exponential(obs=obs, lam=c)

    if kind == 's+b':
        nsig  = zfit.Parameter('nsig', 1000,  0, 10_000)
        nbkg  = zfit.Parameter('nbkg', 1000,  0, 10_000)

        sig   = gauss.create_extended(nsig)
        bkg   = expo.create_extended(nbkg)
        pdf   = zfit.pdf.SumPDF([sig, bkg])

        return pdf

    raise NotImplementedError(f'Invalid kind of fit: {kind}')
#---------------------------------------------
def placeholder_fit(kind : str, fit_dir : str) -> None:
    '''
    Function meant to run toy fits that produce output needed as an input
    to develop tools on top of them

    kind: Kind of fit, e.g. s+b for the simples signal plus background fit
    fit_dir: Directory where the output of the fit will go
    '''

    pdf = _get_model(kind)
    print_pdf(pdf, txt_path=f'{fit_dir}/pre_fit.txt')

    data    = pdf.create_sampler(n=10_000)
    d_const = {'sg' : [0.6, 0.1]}

    obj = Fitter(pdf, data)
    res = obj.fit(cfg={'constraints' : d_const})

    obj   = ZFitPlotter(data=data, model=pdf)
    obj.plot(nbins=50)

    save_fit(data=data, model=pdf, res=res, fit_dir=fit_dir, d_const=d_const)
#---------------------------------------------
