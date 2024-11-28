'''
This script will do checks on the rd_ap_2024 before pushing to feature branch

This problems will be caught before pipelines run
'''
import re

from typing      import Union
from collections import Counter

import argparse
import yaml

from ap_utilities.logging.log_store import LogStore

log = LogStore.add_logger('ap_utilities:check_production')
# --------------------------
class Data:
    '''
    Class storing shared attributes
    '''
    prod_path : str
    regex_info: str = r'"([\w,_,.,-]+)"'
# --------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-p', '--prod_path', type=str, help='Path to directory with production, rd_ap_2024', required=True)
    args = parser.parse_args()

    Data.prod_path = args.prod_path
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
def _sample_from_line(line : str) -> Union[None,str]:
    l_match = re.findall(Data.regex_info, line)
    if len(l_match) != 10:
        return None

    sample = l_match[0]

    return sample
# -------------------------
def _samples_from_info_lines( l_line : list[str]) -> list[str]:
    l_sample = [ _sample_from_line(line) for line   in l_line                        ]
    l_sample = [                  sample for sample in l_sample if sample is not None]

    return l_sample
# -------------------------
def _print_repeated(l_line : list[str]) -> None:
    counter = Counter(l_line)
    l_repeated = [ (element, count) for element, count in counter.items() if count > 1 ]

    for repeated, count in l_repeated:
        log.info(f'{repeated:<40}{count:<10}')
# -------------------------
def _list_to_set(l_line : list[str], msg_repeated : Union[None,str]=None) -> set[str]:
    s_line= set(l_line)
    nlist = len(l_line)
    nset  = len(s_line)

    if nlist != nset and msg_repeated is not None:
        log.error('Repeated elements:')
        _print_repeated(l_line)
        raise ValueError(msg_repeated)

    return s_line
# -------------------------
def _print_set(s_data : set[str], msg : Union[str,None] = None) -> None:
    if len(s_data) == 0:
        return

    log.warning(msg)
    for value in s_data:
        log.info(value)
# -------------------------
def _check_mcdt() -> None:
    d_mcdt        = _load_yaml('tupling/config/mcfuntuple.yaml')
    l_mcdt_sample = list(d_mcdt)
    s_mcdt_sample = _list_to_set(l_mcdt_sample, msg_repeated='Found repeated entries in mcfuntuple')


    l_line = _get_lines('info.yaml')
    l_info_sample = _samples_from_info_lines(l_line)
    s_info_sample = _list_to_set(l_info_sample, msg_repeated='Found repeated entries in info.yaml')

    if s_mcdt_sample != s_info_sample:
        log.warning('Samples in mcfuntuple and info.yaml are different')
        s_mcdt_only = s_mcdt_sample - s_info_sample
        s_info_only = s_info_sample - s_mcdt_sample

        _print_set(s_mcdt_only, msg='MCDecayTree only')
        _print_set(s_info_only, msg='info.yaml only')
# -------------------------
def main():
    '''
    Start of execution
    '''
    _parse_args()
    _check_mcdt()
# -------------------------
if __name__ == '__main__':
    main()
