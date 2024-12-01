'''
Script used to build decay fields from YAML file storing event type -> decay correspondence
'''
import re
import argparse
from typing                         import Union
from importlib.resources            import files

import yaml

import ap_utilities.decays.utilities as aput
from ap_utilities.logging.log_store import LogStore

log = LogStore.add_logger('ap_utilities:make_fields')
# ---------------------------
class Data:
    '''
    Class storing shared data
    '''
    dec_head_rgx = r'\s+(\w+):'

    l_skip_type= [
            '12952000',
            '11453001',
            '13454001',
            '15454101',
            '12442001',
            '11442001',
            '13442001',
            '15444001',
            '15442001',
            '12873002',
            '12425011',
            ]

    d_repl_sym = {
            'cc'        :      'CC',
            '->'        :     '==>',
            }

    d_repl_par = {
            'psi(2S)'    :    'psi_2S_',
            'psi(1S)'    :    'psi_1S_',
            'K*(892)'    :    'K*_892_',
            'phi(1020)'  :  'phi_1020_',
            'K_1(1270)'  :  'K_1_1270_',
            'K_2*(1430)' : 'K_2*_1430_',
            }

    d_repl_spa = {
            '('        :     ' ( ',
            ')'        :     ' ) ',
            '['        :     ' [ ',
            ']'        :     ' ] ',
            }

    l_event_type : list[str]
    d_decay      : dict[str,str]

    d_nicknames = {
            'pi0' : 'pi0',
            'pi+' : 'pip',
            'pi-' : 'pim',
            'X'   : 'X',

            'K+'  : 'Kp',
            'K-'  : 'Km',

            'e+'  : 'Ep',
            'e-'  : 'Em',

            'mu+' : 'Mp',
            'mu-' : 'Mm',

            'tau+': 'taup',
            'tau-': 'taum',

            'p+'  : 'Pp',
            'p~-' : 'Pm',
            'K_S0': 'KS',

            'D-'     : 'D',
            'D0'     : 'D',
            'D0~'    : 'D',
            'D~0'    : 'D',
            'D_s-'   : 'D',
            'D*_s-'  : 'D',
            'nu_tau' : 'nu',
            'nu_mu'  : 'nu',
            'nu_e'   : 'nu',
            'nu_tau~': 'nu',
            'nu_mu~' : 'nu',
            'nu_e~'  : 'nu',

            'B+'  : 'Bu',
            'B-'  : 'Bu',
            'B0'  : 'Bd',
            'X0'  :  'X',
            'B_s0': 'Bs',
            'phi' : 'phi',
            'eta' : 'eta',

            'K_2*(1430)+'    : 'K2',
            'Beauty'         : 'B',
            'K_1+'           : 'K1',
            'K_1(1270)0'     : 'K1',
            'K_1(1270)+'     : 'K1',
            'phi(1020)'      : 'phi',
            'gamma'          : 'gm',
            'K*(892)0'       : 'Kst',
            'K*(892)+'       : 'Kstp',
            'J/psi(1S)'      : 'Jpsi',
            'psi(2S)'        : 'psi2S',
            'Lambda_b0'      : 'Lb',
            'Lambda0'        : 'Lz',
            'Lambda~0'       : 'Lz',
            'Lambda_c-'      : 'Lc',
            'Lambda_c~-'     : 'Lc',
            }
# ---------------------------
def _load_decays() -> None:
    dec_path = files('ap_utilities_data').joinpath('evt_dec.yaml')
    dec_path = str(dec_path)
    with open(dec_path, encoding='utf-8') as ifile:
        Data.d_decay = yaml.safe_load(ifile)
# ---------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-i', '--input'   , type=str, help='Path to textfile with event types')
    parser.add_argument('-l', '--log_lvl' , type=int, help='Logging level', choices=[10,20,30], default=20)
    args = parser.parse_args()

    input_path = args.input
    with open(input_path, encoding='utf-8') as ifile:
        Data.l_event_type = ifile.read().splitlines()

    LogStore.set_level('ap_utilities:make_fields', args.log_lvl)
# ---------------------------
def _reformat_decay(decay : str) -> str:
    # Symbol renaming needed, e.g. -> ==>, cc -> CC
    for org, new in Data.d_repl_sym.items():
        decay = decay.replace(org, new)

    # Need to make special substrings into underscored ones
    # e.g. J/psi(1S) -> J/psi_1S_
    for org, new in Data.d_repl_par.items():
        decay = decay.replace(org, new)

    # Add spaces to parentheses and brackets
    for org, new in Data.d_repl_spa.items():
        decay = decay.replace(org, new)

    # Underscores are part of neutrino names
    # Particles otherwise only use letters
    # Numbers should be excluded, due to anti-D0 -> D~0
    decay = re.sub(r'anti-([a-zA-Z,_]+)', r'\1~', decay)

    return decay
# ---------------------------
def _reformat_back_decay(decay : str) -> str:
    # Put back special characters original naming
    for org, new in Data.d_repl_par.items():
        decay = decay.replace(new, org)

    # Decay cannot have space here, other spaces are allowed
    decay = decay.replace('] CC', ']CC')
    decay = decay.replace('[ '  ,   '[')
    decay = decay.replace('[  ' ,   '[')
    decay = decay.replace('  ]' ,   ']')
    decay = decay.replace('  [' ,   '[')
    decay = decay.replace(' ['  ,   '[')
    decay = decay.replace('(  ' ,   '(')
    decay = decay.replace('  )' ,   ')')

    return decay
# ---------------------------
def _replace_back(part : str) -> str:
    for org, new in Data.d_repl_par.items():
        if new in part:
            part = part.replace(new, org)

    return part
# ---------------------------
def _particles_from_decay(decay : str) -> list[str]:
    l_repl = list(Data.d_repl_sym.values())
    l_repl+= list(Data.d_repl_spa.values())
    l_repl = [ repl.replace(' ', '') for repl in l_repl ]

    l_part = decay.split(' ')
    l_part = [ part for part in l_part if part not in l_repl ]
    l_part = [ part for part in l_part if part != ''         ]
    l_part = [ _replace_back(part) for part in l_part ]

    # Anti-neutrinos and neutrinos will use nu(_index) branch names
    # Need this to make sure neutrinos appear as repeated
    l_part = [ part.rstrip('~') for part in l_part ]

    log.debug(f'Found particles: {l_part}')

    return l_part
# ---------------------------
def _skip_decay(event_type : str, decay : str) -> bool:
    if event_type in Data.l_skip_type:
        log.debug(f'Skipping decay: {decay}')
        return True

    if '{,gamma}' in decay:
        log.warning(f'Skipping {event_type} decay: {decay}')
        return True

    if 'nos' in decay:
        nickname = aput.read_decay_name(event_type=event_type, style= 'safe_1')
        log.warning('Skipping decay:')
        log.info(f'{"":<4}{nickname}')
        log.info(f'{"":<4}{event_type}')
        log.info(f'{"":<4}{decay}')

        return True

    return False
# ---------------------------
def _remove_index(particle : str) -> tuple[str,int]:
    '''
    Takes string representing particle and index of occurence
    Returns particle name and index in a tuple
    '''
    mtch = re.match(r'(.*)_(\d+)$', particle)
    if not mtch:
        return particle, 1

    particle = mtch.group(1)
    npar     = mtch.group(2)
    npar     = int(npar)

    return particle, npar
# ---------------------------
def _get_hatted_decay( particle : str, i_par : int, decay : str) -> str:
    decay = decay.replace(' '   , '  ')
    decay = decay.replace('   ' , '  ')
    decay = decay.replace('    ', '  ')

    if i_par == 0:
        return decay

    particle, ipar = _remove_index(particle)

    decay = _replace_nth_particle(decay, particle, ipar)

    return decay
# ---------------------------
def _move_hat(decay : str) -> str:
    '''
    Up to this point, hats are just behind every particle, including intermediates
    This function will move the hats before parenthesis
    '''
    org_decay = decay
    ihat      = decay.index('^')
    decay     = decay[:ihat]
    decay     = decay.rstrip()
    elm       = decay[-1]

    # Is first non-empty char before hat an opening parenthesis?
    # If not return
    if elm != '(':
        return org_decay

    # Otherwise remove hat from where it is and move it to right place
    ipar      = len(decay) - 1
    org_decay = org_decay.replace('^', ' ')
    decay     = org_decay[:ipar - 1] + '^' + org_decay[ipar:]

    return decay
# ---------------------------
def _replace_nth_particle(decay : str, particle:str, ipar:int) -> str:
    src    = f' {particle}'
    tgt    = f'^{particle}'

    l_part = decay.split(src)
    npart  = len(l_part)
    if npart == 1:
        raise ValueError(f'Cannot find {particle} in {decay}')

    if npart == 2:
        decay = decay.replace(src, tgt)
    else:
        decay = src.join(l_part[:ipar]) + tgt + src.join(l_part[ipar:])

    decay = _move_hat(decay)

    return decay
# ---------------------------
def _rename_repeated(l_par : list[str]) -> list[str]:
    '''
    Takes names of particles
    Returns names of particles, if particles appear more than once, append _x to name
    '''
    d_par_freq = {}
    for par in l_par:
        if par not in d_par_freq:
            d_par_freq[par] = 1
            continue

        d_par_freq[par]+= 1

    l_par_renamed = []
    for par, freq in d_par_freq.items():
        if freq == 1:
            l_par_renamed.append(par)
        else:
            l_par_renamed += [ f'{par}_{i_par}' for i_par in range(1, freq + 1) ]

    return l_par_renamed
# ---------------------------
def _nickname_from_particle(name : str, event_type : str, decname : str) -> str:
    name, ipar = _remove_index(name)
    # Nicknames will be the same for particles and antiparticles
    name       = name.replace('anti-', '')

    if name not in Data.d_nicknames:
        log.warning(f'Nickname for {name} not found in {decname}/{event_type}')
        return name

    nick = Data.d_nicknames[name]
    if ipar > 1:
        nick = f'{nick}_{ipar}'

    return nick
# ---------------------------
def _fix_beauty(decay : str, event_type : str) -> str:
    if 'Beauty' not in decay:
        return decay

    if event_type == '11102453':
        bname = 'B0'
    else:
        log.warning(f'Cannot identify B meson type for {event_type}')
        bname = 'Beauty'

    decay = decay.replace('Beauty', bname)

    return decay
# ---------------------------
def _fix_phi(decay : str) -> str:
    rgx   = r'phi(?!\s*\(\s*1020\s*\)\s*)'
    decay = re.sub(rgx, 'phi(1020)', decay)

    return decay
# ---------------------------
def _fix_names(decay : str, event_type : str) -> str:
    '''
    Decay field in decay files is not properly written, need to fix here, before using decay
    '''
    decay = decay.replace('K_1+' ,  'K_1(1270)+')
    decay = decay.replace('K*+'  ,    'K*(892)+')
    decay = decay.replace('K*0'  ,    'K*(892)0')
    decay = decay.replace('D_s*' ,        'D*_s')
    decay = decay.replace('My_'  ,            '')
    decay = _fix_phi(decay)
    decay = _fix_beauty(decay, event_type)

    return decay
# ---------------------------
def _get_decay(event_type : str, decname : str) -> Union[None,dict[str,str]]:
    decay = Data.d_decay[event_type]
    decay = _fix_names(decay, event_type)

    if _skip_decay(event_type, decay):
        return None

    decay = _reformat_decay(decay)
    l_par = _particles_from_decay(decay)
    l_par = _rename_repeated(l_par)
    decay = _reformat_back_decay(decay)
    decay = _check_unhatted_decay(decay, event_type)

    d_dec = {}
    for i_par, par in enumerate(l_par):
        nickname        = _nickname_from_particle(par, event_type, decname)
        d_dec[nickname] = _get_hatted_decay(par, i_par, decay)

    return d_dec
# ---------------------------
def _check_closed(decay : str, left : str, right : str) -> None:
    nlft = decay.count( left)
    nrgt = decay.count(right)

    if nlft != nrgt:
        log.error(f'Failed closure {left}{right} in {decay}')
# ---------------------------
def _check_unhatted_decay(decay : str, event_type : str) -> str:
    '''
    Final check of decay, will apply fixes if needed
    '''
    if not decay.endswith('CC'):
        log.warning(f'Decay {decay}/{event_type} had no conjugate adding it')
        return f'{decay}CC'

    _check_closed(decay, '(', ')')
    _check_closed(decay, '[', ']')

    return decay
# ---------------------------
def _get_decays() -> dict[str, dict[str,str]]:
    d_decay = {}
    for event_type in Data.l_event_type:
        decname = aput.read_decay_name(event_type=event_type, style= 'safe_1')
        d_tmp   = _get_decay(event_type, decname)
        if d_tmp is None:
            continue

        d_decay[decname] = d_tmp

    return d_decay
# ---------------------------
def _remove_ending_spaces(line : str) -> str:
    '''
    In the lines with a quote at the end, do:

    ']   ' -> ']'
    '''
    if '\'' not in line:
        return line

    line = line.rstrip()
    line = line.rstrip('\'')
    line = line.rstrip()

    return f'{line}' + '\''
# ---------------------------
def _format_yaml_line(line : str) -> str:
    line = _remove_ending_spaces(line)

    mtch = re.match(Data.dec_head_rgx, line)
    if not mtch:
        return line

    head     = mtch.group(1)
    head_pad = f'{head:5}'

    line = line.replace(f'{head}:', f'{head_pad}:')

    return line
# ---------------------------
def _save_decays(path : str, d_decay : dict[str,dict[str,str]]) -> None:
    with open(path, 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_decay, ofile, width=200)

    with open(path, encoding='utf-8') as ifile:
        l_line = ifile.read().splitlines()

    l_line = [ _format_yaml_line(line) for line in l_line ]

    text = '\n'.join(l_line)
    with open(path, 'w', encoding='utf-8') as ofile:
        ofile.write(text)
# ---------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _load_decays()

    d_decay = _get_decays()
    _save_decays('decays.yaml', d_decay)
# ---------------------------
if __name__ == '__main__':
    main()
