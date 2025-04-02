'''
Script used to link ntuples properly and merge them
'''

# pylint: disable=line-too-long, import-error
# pylint: disable=invalid-name

import re
import os
import glob
import json
import argparse
from typing                 import Union
from dataclasses            import dataclass
from importlib.resources    import files

import yaml
import dmu.generic.utilities as gut
from dmu.rfile.rfprinter    import RFPrinter
from dmu.logging.log_store  import LogStore
from rx_data.path_splitter  import PathSplitter

log   = LogStore.add_logger('rx_data:make_tree_structure')
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
    grid_preffix = 'root://x509up_u12477@eoslhcb.cern.ch//eos/lhcb/grid/user'
    eos_preffix  = 'root://eosuser.cern.ch/'
    l_line_to_pick : list[str]

    naming    : str
    kind      : str
    max_files : int
    new_path  : str
    ver       : str
    dry       : bool
    jsn_ver   : str
    inp_path  : str
    out_path  : str
    fil_path  : str
# ---------------------------------
def _get_paths_from_filesystem() -> list[str]:
    '''
    Returns list of paths to ROOT files corresponding to a given job
    '''
    path_wc = f'{Data.inp_path}/*.root'
    l_path  = glob.glob(path_wc)

    npath   = len(l_path)
    if npath == 0:
        log.error(f'No file found in: {path_wc}')
        raise FileNotFoundError

    log.info(f'Found {npath} paths')

    return l_path
# ---------------------------------
def _get_paths_from_json() -> list[str]:
    if Data.kind is None:
        raise ValueError('Kind argument not passed')

    jsn_wc = files('rx_data_lfns').joinpath(f'{Data.kind}/{Data.jsn_ver}/*.json')
    jsn_wc = str(jsn_wc)
    l_path = glob.glob(jsn_wc)

    npath  = len(l_path)
    if npath == 0:
        raise FileNotFoundError(f'No files found in {jsn_wc}')

    l_lfn  = []
    for path in l_path:
        with open(path, encoding='utf-8') as ifile:
            l_this_file = json.load(ifile)
            l_this_file = [ f'{Data.grid_preffix}{this_file}' for this_file in l_this_file ]
            l_lfn      += l_this_file

    return l_lfn
# ---------------------------------
def _get_paths() -> list[str]:
    if Data.jsn_ver is not None:
        return _get_paths_from_json()

    if Data.inp_path is not None:
        return _get_paths_from_filesystem()

    raise ValueError('Cannot get paths, either filesystem path nor JSON version specified')
# ---------------------------------
def _link_paths(sample : str, line : str, l_path : list[str]) -> Union[str, None]:
    '''
    Makes symbolic links of list of paths of a specific kind
    info is a tuple with = (sample, channel, kind, year) information
    Will return directory where linked files are
    '''
    if Data.out_path is None:
        return None

    npath = len(l_path)
    log.debug(f'Linking {npath} paths for {sample}/{line}')

    target_dir  = f'{Data.out_path}/{Data.ver}/post_ap/{sample}/{line}'
    os.makedirs(target_dir, exist_ok=True)

    log.debug(f'Linking to: {target_dir}')
    if Data.dry:
        log.warning('Dry run, not linking')
        return None

    for source_path in l_path:
        file_name   = os.path.basename(source_path)
        target_path = f'{target_dir}/{file_name}'

        log.debug(f'{source_path:<50}{"->":10}{target_path:<50}')
        _do_link_paths(src=source_path, tgt=target_path)

    return target_dir
# ---------------------------------
def _do_link_paths(src : str, tgt : str) -> None:
    '''
    Will check if target link exists, will delete it if it does
    Will make link
    '''
    if os.path.exists(tgt):
        os.unlink(tgt)

    os.symlink(src, tgt)
# ---------------------------------
@gut.timeit
def _save_summary(target_dir : str) -> None:
    '''
    Make text file with summary of file, e.g. 2024.root -> 2024.txt
    '''
    if Data.dry:
        return

    l_file_path = glob.glob(f'{target_dir}/*.root')
    if len(l_file_path) == 0:
        log.warning(f'No ROOT file found in {target_dir}')
        return

    target_file = l_file_path[0]

    prt = RFPrinter(path=target_file)
    prt.save(file_name='summary.txt', raise_on_fail=False)
# ---------------------------------
def _get_args() -> argparse.Namespace:
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Makes directory structure from ROOT files through symbolic links')
    parser.add_argument('-i', '--inp' , type=str, help='Path to directory with ROOT files to link')
    parser.add_argument('-v', '--ver' , type=str, help='Version of LFNs needed to pick up JSON files')
    parser.add_argument('-o', '--out' , type=str, help='Path to directory where tree structure will start')
    parser.add_argument('-f', '--fle' , type=str, help='Path to YAML file with directory structure')
    parser.add_argument('-t', '--trg' , type=str, help='Path to YAML file with list of lines to process')
    parser.add_argument('-p', '--path', type=str, help='Path to files that will override original one, e.g. in EOS.')
    parser.add_argument('-k', '--kind', type=str, help='Type of production', choices=['rx', 'lbpkmumu'])
    parser.add_argument('-n', '--nam' , type=str, help='Naming scheme for samples', default='new', choices=['new', 'old'])
    parser.add_argument('-m', '--max' , type=int, help='Maximum number of paths, for test runs'   , default=-1)
    parser.add_argument('-l', '--lvl' , type=int, help='log level', choices=[10, 20, 30]          , default=20)
    parser.add_argument('-d', '--dry' ,           help='Dry run if 1', action='store_true')
    args = parser.parse_args()

    return args
# ---------------------------------
def _version_from_input() -> Union[str,None]:
    if Data.inp_path is None:
        return None

    version = os.path.basename(Data.inp_path)
    if not re.match(r'v\d+', version):
        raise ValueError(f'Cannot extract version from: {version}')

    log.info(f'Using version {version}')

    return version
# ---------------------------------
def _load_lines(args : argparse.Namespace) -> list[str]:
    if args.trg is None:
        return []

    path = args.trg
    log.debug(f'Picking up lines from: {path}')
    with open(path, encoding='utf-8') as ifile:
        d_trig = yaml.safe_load(ifile)

    return list(d_trig)
# ---------------------------------
@gut.timeit
def _initialize(args : argparse.Namespace) -> None:
    Data.dry       = args.dry
    Data.kind      = args.kind
    Data.naming    = args.nam
    Data.max_files = args.max
    Data.inp_path  = args.inp
    Data.jsn_ver   = args.ver
    Data.out_path  = args.out
    Data.fil_path  = args.fle
    Data.new_path  = args.path

    LogStore.set_level('rx_data:make_tree_structure', args.lvl)
    LogStore.set_level('rx_data:path_splitter'      , args.lvl)
    LogStore.set_level('dmu:rfprinter', 30)

    Data.ver            = _version_from_input()
    Data.l_line_to_pick = _load_lines(args)
    gut.TIMER_ON        = args.lvl < 20
# ---------------------------------
def _change_file_paths(d_struc : dict) -> dict:
    if Data.new_path is None:
        log.debug('Not changing paths')
        return d_struc

    log.info(f'Overriding paths with: {Data.new_path}')
    d_struc_mod = {}
    for sample, d_trig in d_struc.items():
        d_struc_mod[sample] = {}
        for trigger, l_path in d_trig.items():
            l_path_mod = [ _change_file_path(path) for path in l_path ]
            d_struc_mod[sample].update({trigger : l_path_mod})

    return d_struc_mod
# ---------------------------------
def _change_file_path(path : str) -> str:
    file_name = os.path.basename(path)

    return f'{Data.eos_preffix}{Data.new_path}/{file_name}'
# ---------------------------------
def _sort_lists(d_struc : dict) -> None:
    for d_trig in d_struc.values():
        for l_path in d_trig.values():
            l_path.sort()
# ---------------------------------
@gut.timeit
def _save_to_file(d_struc : dict) -> None:
    if Data.fil_path is None:
        return

    if Data.dry:
        return

    out_dir = os.path.dirname(Data.fil_path)
    if out_dir != '':
        os.makedirs(out_dir, exist_ok=True)

    d_struc = _change_file_paths(d_struc)

    log.info(f'Saving samples list to: {Data.fil_path}')
    with open(Data.fil_path, 'w', encoding='utf-8') as ofile:
        _sort_lists(d_struc)
        yaml.dump(d_struc, ofile, Dumper=IndentListDumper, default_flow_style=False)
# ---------------------------------
def _drop_line(line_name : str) -> bool:
    if len(Data.l_line_to_pick) == 0:
        return False

    if line_name not in Data.l_line_to_pick:
        return True

    return False
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    args = _get_args()
    _initialize(args)

    l_path = _get_paths()
    splt   = PathSplitter(paths=l_path, max_files=Data.max_files, sample_naming=Data.naming)
    d_path = splt.split()

    d_struc = {}
    for (sample, line), l_path in d_path.items():
        if _drop_line(line):
            log.debug(f'Dropping {line}')
            continue

        if sample not in d_struc:
            d_struc[sample] = {}

        d_struc[sample][line] = l_path

        target_dir = _link_paths(sample, line, l_path)
        if target_dir is not None:
            _save_summary(target_dir)

    _save_to_file(d_struc)
# ---------------------------------
if __name__ == '__main__':
    main()
