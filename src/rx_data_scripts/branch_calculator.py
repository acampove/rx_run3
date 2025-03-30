'''
Script used to create small trees with extra branches from input trees
'''

# pylint: disable=line-too-long, import-error
# pylint: disable=invalid-name
# pylint: disable=broad-exception-caught

import os
import glob
import fnmatch
import argparse
from dataclasses import dataclass

import tqdm
from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from dmu.generic            import version_management as vman

from rx_data.mis_calculator      import MisCalculator
from rx_data.hop_calculator      import HOPCalculator
from rx_data.swp_calculator      import SWPCalculator
from rx_data.mass_bias_corrector import MassBiasCorrector

log = LogStore.add_logger('rx_data:branch_calculator')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    vers : str
    kind : str
    nmax : int
    part : tuple[int,int]
    pbar : bool
    dry  : bool
    lvl  : int
    wild_card : str

    l_kind    = ['hop', 'swp_jpsi_misid', 'swp_cascade', 'ecalo_bias', 'brem_track_1', 'brem_track_2']
    l_ecorr   = ['ecalo_bias', 'brem_track_1', 'brem_track_2']
    tree_name = 'DecayTree'
# ---------------------------------
def _parse_args() -> None:
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Script used to create ROOT files with trees with extra branches by picking up inputs from directory and patitioning them')
    parser.add_argument('-k', '--kind', type=str, help='Kind of branch to create', choices=Data.l_kind, required=True)
    parser.add_argument('-v', '--vers', type=str, help='Version of outputs', required=True)
    parser.add_argument('-w', '--wc'  , type=str, help='Wildcard, if passed will be used to match paths')
    parser.add_argument('-n', '--nmax', type=int, help='If used, limit number of entries to process to this value')
    parser.add_argument('-p', '--part', nargs= 2, help='Partitioning, first number is the index, second is the number of parts', required=True)
    parser.add_argument('-b', '--pbar',           help='If used, will show progress bar whenever it is available', action='store_true')
    parser.add_argument('-d', '--dry' ,           help='If used, will do dry drun, e.g. stop before processing', action='store_true')
    parser.add_argument('-l', '--lvl' , type=int, help='log level', choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.kind = args.kind
    Data.vers = args.vers
    Data.part = args.part
    Data.nmax = args.nmax
    Data.pbar = args.pbar
    Data.dry  = args.dry
    Data.lvl  = args.lvl
    Data.wild_card = args.wc

    LogStore.set_level('rx_data:branch_calculator', Data.lvl)
# ---------------------------------
def _get_path_size(path : str) -> int:
    path = os.path.realpath(path)
    size = os.path.getsize(path)
    size = size / 1024 ** 2
    size = int(size)

    return size
# ---------------------------------
def _get_partition(l_path : list[str]) -> list[str]:
    igroup, ngroup = Data.part
    igroup = int(igroup)
    ngroup = int(ngroup)

    d_path      = { path : _get_path_size(path) for path in l_path }
    sorted_files= sorted(d_path.items(), key=lambda x: x[1], reverse=True)

    groups      = {i: [] for i in range(ngroup)}
    group_sizes = {i: 0  for i in range(ngroup)}

    for file_path, size in sorted_files:
        min_group = min(group_sizes, key=group_sizes.get)
        groups[min_group].append(file_path)
        group_sizes[min_group] += size

    _print_groups(groups, group_sizes, igroup)

    norg  = len(l_path)
    l_path= groups[igroup]
    npar  = len(l_path)
    log.info(f'Processing group of {npar} files out of {norg}')
    for path in l_path:
        log.debug(path)

    return l_path
# ---------------------------------
def _print_groups(group : dict[int,list[str]], sizes : dict[int,float], this_group : int) -> None:
    log.info(30 * '-')
    log.info(f'{"Group":<10}{"NFiles":<10}{"Size":<10}')
    log.info(30 * '-')
    for igroup, l_file in group.items():
        size  = sizes[igroup]
        nfile = len(l_file)

        if igroup == this_group:
            log.info(f'{igroup:<10}{nfile:<10}{size:<10}{"<---":<10}')
        else:
            log.info(f'{igroup:<10}{nfile:<10}{size:<10}')

    log.info(30 * '-')
# ---------------------------------
def _filter_paths(l_path : list[str]) -> list[str]:
    ninit = len(l_path)
    log.debug(f'Filtering {ninit} paths')
    if Data.kind in Data.l_ecorr:
        # For electron corrections, drop muon paths
        l_path = [ path for path in l_path if 'MuMu' not in path ]

    if Data.wild_card is not None:
        l_path = [ path for path in l_path if fnmatch.fnmatch(path, f'*{Data.wild_card}*') ]

    nfnal = len(l_path)
    log.debug(f'Filtered -> {nfnal} paths')

    return l_path
# ---------------------------------
def _get_paths() -> list[str]:
    data_dir = os.environ['DATADIR']
    data_dir = vman.get_last_version(dir_path=f'{data_dir}/main', version_only=False)
    l_path   = glob.glob(f'{data_dir}/*.root')
    l_path   = _filter_paths(l_path)
    l_path   = _get_partition(l_path)

    nfiles   = len(l_path)
    if nfiles == 0:
        raise ValueError(f'No file found in: {data_dir}')

    log.info(f'Picking up {nfiles} file(s) from {data_dir}')

    return l_path
# ---------------------------------
def _get_out_dir() -> str:
    out_dir  = os.environ['DATADIR']
    out_dir  = f'{out_dir}/{Data.kind}/{Data.vers}'

    if not Data.dry:
        os.makedirs(out_dir, exist_ok=True)

    return out_dir
# ---------------------------------
def _get_out_path(path : str) -> str:
    fname    = os.path.basename(path)
    out_path = f'{Data.out_dir}/{fname}'

    log.debug(f'Creating : {out_path}')

    return out_path
# ---------------------------------
def _is_mc(path : str) -> bool:
    if '/data_24_mag' in path:
        return False

    if '/mc_mag' in path:
        return True

    raise ValueError(f'Cannot determine if MC or data for: {path}')
# ---------------------------------
def _create_file(path : str, trigger : str) -> None:
    out_path = _get_out_path(path)
    if os.path.isfile(out_path):
        log.debug(f'Output found, skipping {out_path}')
        return

    if Data.dry:
        return

    rdf = RDataFrame(Data.tree_name, path)
    nentries = rdf.Count().GetValue()
    if nentries == 0:
        log.warning(f'Found empty input file: {path}/{Data.tree_name}')
        rdf=RDataFrame(0)
        rdf=rdf.Define('fake_column', '1')
        rdf.Snapshot(Data.tree_name, out_path)
        return

    if Data.nmax is not None:
        log.warning(f'Limitting dataframe to {Data.nmax} entries')
        rdf=rdf.Range(Data.nmax)

    msc = MisCalculator(rdf=rdf, trigger=trigger)
    rdf = msc.get_rdf()

    # TODO: Remove the SS condition for the SWPCalculator
    # When the data ntuples with fixed descriptor be ready
    is_ss = 'SameSign' in trigger

    if   Data.kind == 'hop':
        obj = HOPCalculator(rdf=rdf)
        rdf = obj.get_rdf(preffix=Data.kind)
    elif Data.kind in Data.l_ecorr:
        skip_correction = _is_mc(path) and Data.kind == 'ecalo_bias'
        if skip_correction:
            log.warning('Turning off ecalo_bias correction for MC sample')

        cor = MassBiasCorrector(rdf=rdf, skip_correction=skip_correction, ecorr_kind=Data.kind)
        rdf = cor.get_rdf(suffix=Data.kind)
    elif Data.kind == 'swp_jpsi_misid':
        obj = SWPCalculator(rdf=rdf, d_lep={'L1' :  13, 'L2' :  13}, d_had={'H' :  13})
        rdf = obj.get_rdf(preffix=Data.kind, progress_bar=Data.pbar, use_ss=is_ss)
    elif Data.kind == 'swp_cascade'   :
        obj = SWPCalculator(rdf=rdf, d_lep={'L1' : 211, 'L2' : 211}, d_had={'H' : 321})
        rdf = obj.get_rdf(preffix=Data.kind, progress_bar=Data.pbar, use_ss=is_ss)
    else:
        raise ValueError(f'Invalid kind: {Data.kind}')

    rdf.Snapshot(Data.tree_name, out_path)
# ---------------------------------
def _trigger_from_path(path : str) -> str:
    ichar   = path.index('Hlt2')
    fchar   = path.index('_MVA') + 4

    if ichar >= fchar:
        raise ValueError(f'Cannot extract trigger name from: {path}')

    trigger = path[ichar:fchar]

    log.debug(f'Using trigger: {trigger}')

    return trigger
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()

    l_path       = _get_paths()
    Data.out_dir = _get_out_dir()
    for path in tqdm.tqdm(l_path, ascii=' -'):
        trigger = _trigger_from_path(path)
        _create_file(path, trigger)
# ---------------------------------
if __name__ == '__main__':
    main()
