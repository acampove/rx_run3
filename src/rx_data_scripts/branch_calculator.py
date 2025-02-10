'''
Script used to create small trees with extra branches from input trees
'''

# pylint: disable=line-too-long, import-error
# pylint: disable=invalid-name

import os
import argparse
from dataclasses import dataclass

import yaml
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:branch_calculator')
# ---------------------------------
class IndentListDumper(yaml.SafeDumper):
    '''
    Class needed to implement indentation correctly in dumped yaml files
    '''
    def increase_indent(self, flow=False, indentless=False):
        return super().increase_indent(flow, False)
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    path : str
    outp : str
    kind : str
    lvl  : int
# ---------------------------------
def _parse_args() -> None:
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Script used to create ROOT files with trees with extra branches')
    parser.add_argument('-p', '--path', type=str, help='Path to YAML file', required=True)
    parser.add_argument('-o', '--outp', type=str, help='Path to directory with outputs', required=True)
    parser.add_argument('-k', '--kind', type=str, help='Kind of branch to create', choices=['hop'], required=True)
    parser.add_argument('-l', '--lvl' , type=int, help='log level', choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.path = args.path
    Data.outp = args.outp
    Data.kind = args.kind
    Data.lvl  = args.lvl
# ---------------------------------
def _get_paths() -> list[str]:
    with open(Data.path, encoding='utf-8') as ifile:
        d_sample = yaml.safe_load(ifile)

    l_path = []
    for sample in d_sample:
        for trigger in d_sample[sample]:
            l_path += d_sample[sample][trigger]

    nfile = len(l_path)
    log.info(f'Found {nfile} files')

    return l_path
# ---------------------------------
def _create_file(path : str) -> None:
    fname    = os.path.basename(path)
    out_path = f'{Data.outp}/{fname}'

    if os.path.isfile(out_path):
        log.info(f'Output found, skipping {out_path}')

    log.info(f'Creating {out_path}')

# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    os.makedirs(Data.outp, exist_ok=True)

    l_path = _get_paths()

    for path in l_path:
        _create_file(path)
# ---------------------------------
if __name__ == '__main__':
    main()
