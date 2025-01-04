'''
Module that will transform lists of LFNs from Ganga post_ap jobs
into a yaml file ready to be plugged into the RX c++ framework
'''
# pylint: disable=line-too-long, import-error

import glob
import json
import argparse

from importlib.resources    import files
from dataclasses            import dataclass

import yaml
from dmu.logging.log_store  import LogStore
from rx_data.path_splitter import PathSplitter

log = LogStore.add_logger('rx_data:make_tree_structure')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    # pylint: disable = invalid-name

    max_files : int
    lfn_vers  : str
    l_project = ['RK', 'RKst']
# ---------------------------------
def _get_paths() -> list[str]:
    '''
    Returns list of LFNs, taken from data stored in project itself, rx_data_lfns
    '''
    files_wc = files('rx_data_lfns').joinpath(f'{Data.lfn_vers}/*.json')
    files_wc = str(files_wc)
    l_file   = glob.glob(files_wc)
    nfile    = len(l_file)

    if nfile == 0:
        raise ValueError(f'Cannot find any file in: {files_wc}')

    l_path   = []

    for file in l_file:
        with open(file, encoding='utf-8') as ifile:
            l_path_file = json.load(ifile)
            nlfn        = len(l_path_file)
            log.debug(f'Adding {nlfn} LFNs')
            l_path     += l_path_file

    return l_path
# ---------------------------------
def _get_args() -> argparse.Namespace:
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Will make YAML files with specific formatting from lists of LFNs in project')
    parser.add_argument('-m', '--max', type=int, help='Maximum number of paths, for test runs'   , default =-1)
    parser.add_argument('-v', '--ver', type=str, help='Version of LFNs'                          , required=True)
    parser.add_argument('-l', '--lvl', type=int, help='log level', choices=[10, 20, 30]          , default =20)
    args = parser.parse_args()

    return args
# ---------------------------------
def _initialize(args : argparse.Namespace) -> None:
    Data.max_files = args.max
    Data.lfn_vers  = args.ver

    LogStore.set_level('rx_data:lfn_to_yaml', args.lvl)
# ---------------------------------
def _save(d_path : dict) -> None:
    for key in d_path:
        print(key)

    return
    with open('samples.yaml', 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_path, ofile)
# ---------------------------------
def _reformat_paths(d_path : dict[tuple[str,str],list[str]]) -> dict[str,dict[str,list[str]]]:
    d_sample = {}
    for (sample, trigger), l_path in d_path.items():
        if sample not in d_sample:
            d_sample[sample] = {}

        d_sample[sample][trigger] = l_path

    return d_sample
# ---------------------------------
def _filter_samples()
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    args = _get_args()
    _initialize(args)

    l_path = _get_paths()
    splt   = PathSplitter(paths=l_path, max_files=Data.max_files)
    d_path = splt.split()

    d_sample = _reformat_paths(d_path)
    d_sample = _filter_samples(d_sample)

    _save(d_sample)
# ---------------------------------
if __name__ == '__main__':
    main()
