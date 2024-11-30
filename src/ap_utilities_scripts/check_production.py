'''
This script will do checks on the rd_ap_2024 before pushing to feature branch

This problems will be caught before pipelines run
'''
import re

from typing      import Union
from collections import Counter

import argparse
import yaml

import ap_utilities.io.utilities    as iout 
from ap_utilities.logging.log_store import LogStore

log = LogStore.add_logger('ap_utilities:check_production')
# --------------------------
class Data:
    '''
    Class storing shared attributes
    '''
    prod_path   : str
    l_skip_subs : Union[list[str],None]
    regex_info  : str = r'"([\w,_,.,-]+)"'
    d_samples   : dict[str, set[str]]            = {}
    l_info      : list[list[str]]                = []
    d_report    : dict                           = {'missing' : {}}
# --------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-p', '--prod_path', type=str , help='Path to directory with production, rd_ap_2024'          , required= True)
    parser.add_argument('-s', '--skip_subs', nargs='+', help='If nickname contains this substring, it will be skipped', required=False)
    args = parser.parse_args()

    Data.prod_path   = args.prod_path
    Data.l_skip_subs = args.skip_subs
# -------------------------
def _load_yaml(name : str) -> dict:
    path = f'{Data.prod_path}/{name}'
    with open(path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# -------------------------
def _get_lines(name : str) -> list[str]:
    path = f'{Data.prod_path}/{name}'
    with open(path, encoding='utf-8') as ifile:
        l_line = ifile.read().splitlines()

    return l_line
# -------------------------
def _sample_from_line(line : str) -> Union[None,list[str]]:
    '''
    Takes line from info.yaml
    Returns a list of strings, each with configuration for an MC sample, if line is not present, returns None
    '''
    l_match = re.findall(Data.regex_info, line)
    if len(l_match) != 10:
        return None

    return l_match
# -------------------------
def _sample_from_info(l_info_line : str) -> list[list[str]]:
    '''
    Takes list of lines in info.yaml
    Returns list of sets of strings defining MC samples
    '''
    l_retval   = [ _sample_from_line(line) for line     in l_info_line                        ]
    l_l_config = [ l_config                for l_config in l_retval    if l_config is not None]

    return l_l_config
# -------------------------
def _samples_from_info_lines( l_line : list[str]) -> list[str]:
    l_retval  = [ _sample_from_line(line) for line   in l_line                        ]
    l_sample  = [ retval[0]               for retval in l_retval if retval is not None]

    return l_sample
# -------------------------
def _print_repeated(l_line : list[str]) -> None:
    counter = Counter(l_line)
    l_repeated = [ (element, count) for element, count in counter.items() if count > 1 ]

    for repeated, count in l_repeated:
        log.info(f'{repeated:<40}{count:<10}')
# -------------------------
def _skip_sample(nickname : str) -> bool:
    if Data.l_skip_subs is None:
        return False

    for subs in Data.l_skip_subs:
        if subs in nickname:
            return True

    return False
# -------------------------
def _list_to_set(l_line : list[str], msg_repeated : Union[None,str]=None) -> set[str]:
    l_line = [ line for line in l_line if not _skip_sample(line) ]
    s_line = set(l_line)
    nlist  = len(l_line)
    nset   = len(s_line)

    if nlist != nset and msg_repeated is not None:
        log.error('Repeated elements:')
        _print_repeated(l_line)
        raise ValueError(msg_repeated)

    return s_line
# -------------------------
def _load_samples() -> None:
    l_info_line   = _get_lines('info.yaml')
    Data.l_info   = _sample_from_info(l_info_line)

    l_info_sample = _samples_from_info_lines(l_info_line)
    s_info_sample = _list_to_set(l_info_sample, msg_repeated='Found repeated entries in info.yaml')

    d_mcdt        = _load_yaml('tupling/config/mcfuntuple.yaml')
    l_mcdt_sample = list(d_mcdt)
    s_mcdt_sample = _list_to_set(l_mcdt_sample, msg_repeated='Found repeated entries in mcfuntuple')

    d_samp        = _load_yaml('tupling/config/samples.yaml')
    l_samp_sample = list(d_samp)
    s_samp_sample = _list_to_set(l_samp_sample, msg_repeated='Found repeated entries in samples.yaml')

    Data.d_samples['info.yaml'      ] = s_info_sample
    Data.d_samples['mcfuntuple.yaml'] = s_mcdt_sample
    Data.d_samples['samples.yaml'   ] = s_samp_sample
# -------------------------
def _get_difference(s_val1 : set[str], s_val2 : set[str]) -> list[str]:
    s_diff = s_val1 - s_val2
    l_diff = list(s_diff)
    l_diff.sort()

    return l_diff
# -------------------------
def _check_samples(name_1 : str, name_2 : str) -> None:
    s_sample_1 = Data.d_samples[name_1]
    s_sample_2 = Data.d_samples[name_2]

    d_sample   = {}
    if s_sample_1 != s_sample_2:
        log.warning(f'Samples in {name_1} and {name_2} are different')

        d_sample[f'only {name_1}'] = _get_difference(s_sample_1, s_sample_2)
        d_sample[f'only {name_2}'] = _get_difference(s_sample_2, s_sample_1)

    Data.d_report['missing'][f'{name_1}_{name_2}'] = d_sample
# -------------------------
def _compare_groups() -> None:
    _check_samples('info.yaml'      , 'mcfuntuple.yaml')
    _check_samples('info.yaml'      , 'samples.yaml'   )
    _check_samples('mcfuntuple.yaml', 'samples.yaml'   )
# -------------------------
def _save_report():
    out_path = 'report.yaml'

    with open(out_path, 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(Data.d_report, ofile, width=200)

    iout.reformat_yaml(path = out_path)
# -------------------------
def _check_name_lenghts() -> None:
    l_long_nickname = []
    for [nickname, evt_type, mc_path, polarity, _, _, _, nuval, sim_version, generator] in Data.l_info:
        mc_path  = mc_path.replace('.', '_')
        job_name = f'MC_{mc_path}_{polarity}_{nuval}_{sim_version}_{generator}_{evt_type}_{nickname}'
        size     = len(job_name)
        if size > 100:
            log.warning(f'{size:<20}{nickname:<100}')
            size = str(size)
            l_long_nickname.append([nickname, size])

    Data.d_report['long_nicknames'] = l_long_nickname
# -------------------------
def main():
    '''
    Start of execution
    '''
    _parse_args()
    _load_samples()
    _compare_groups()
    _check_name_lenghts()

    _save_report()
# -------------------------
if __name__ == '__main__':
    main()
