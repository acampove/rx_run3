'''
Module with utility functions related to the dmu.stats project
'''
import os
import re
from typing import Union
import zfit

from dmu.logging.log_store import LogStore

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
        val      = f'{mu:.3e}; {sg:.3e}'
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
#-------------------------------------------------------
