'''
Script used to create small trees with extra branches from input trees
'''

# pylint: disable=line-too-long, import-error
# pylint: disable=invalid-name

import os
import argparse
from typing      import Union
from dataclasses import dataclass

import tqdm
import yaml
from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_data.hop_calculator import HOPCalculator

log = LogStore.add_logger('rx_data:branch_calculator')
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
    l_kind    = ['hop']
    tree_name = 'DecayTree'
# ---------------------------------
def _parse_args() -> None:
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Script used to create ROOT files with trees with extra branches')
    parser.add_argument('-p', '--path', type=str, help='Path to YAML file', required=True)
    parser.add_argument('-o', '--outp', type=str, help='Path to directory with outputs', required=True)
    parser.add_argument('-k', '--kind', type=str, help='Kind of branch to create', choices=Data.l_kind, required=True)
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
def _get_out_path(path : str) -> Union[str,None]:
    fname    = os.path.basename(path)
    out_path = f'{Data.outp}/{fname}'

    if os.path.isfile(out_path):
        log.debug(f'Output found, skipping {out_path}')
        return None

    log.debug(f'Creating {out_path}')
    return out_path
# ---------------------------------
def _create_file(path : str) -> None:
    out_path = _get_out_path(path)
    if out_path is None:
        return

    rdf = RDataFrame(Data.tree_name, path)
    obj = HOPCalculator(rdf=rdf)
    rdf = obj.get_rdf(extra_branches=['EVENTNUMBER', 'RUNNUMBER'])
    rdf.Snapshot(Data.tree_name, out_path)
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    os.makedirs(Data.outp, exist_ok=True)

    l_path = _get_paths()

    for path in tqdm.tqdm(l_path, ascii=' -'):
        _create_file(path)
# ---------------------------------
if __name__ == '__main__':
    main()
