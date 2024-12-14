#!/usr/bin/python3
'''
Script used to link ntuples properly and merge them
'''

import re
import os
import glob
import pprint
import argparse

from typing                 import Union
from dataclasses            import dataclass
from functools              import cache
from importlib.resources    import files
from dmu.rfile.rfprinter    import RFPrinter
from dmu.logging.log_store  import LogStore

import tqdm
import yaml

from ROOT                   import TFileMerger, TTree

log = LogStore.add_logger('post_ap:link_merge')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    # pylint: disable = invalid-name
    # Need to call var Max instead of max

    job     : str
    dry     : int
    Max     : int
    ver     : str
    cfg_dat : dict

    inp_dir = '/publicfs/lhcb/user/campoverde/Data/RK'
    dt_rgx  = r'(?:dt|data)_(\d{4}|\d{2}).*tuple_Hlt2RD_(.*)\.root'
    mc_rgx  = r'mc_.*_(\d{8})_nu.*tuple_Hlt2RD_(.*)\.root'
# ---------------------------------
def _get_args():
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-j', '--job', type=str, help='Job name, e.g. flt_001', required=True)
    parser.add_argument('-d', '--dry', type=int, help='Dry run if 1', choices=[0, 1], default=0)
    parser.add_argument('-l', '--lvl', type=int, help='log level', choices=[10, 20, 30], default=20)
    parser.add_argument('-m', '--max', type=int, help='Maximum number of paths, for test runs', default=-1)
    parser.add_argument('-v', '--ver', type=str, help='Version used to name outputs', required=True)
    args = parser.parse_args()

    Data.job = args.job
    Data.dry = args.dry
    Data.Max = args.max
    Data.ver = args.ver

    log.setLevel(args.lvl)
# ---------------------------------
def _get_paths():
    '''
    Returns list of paths to ROOT files corresponding to a given job
    '''
    path_wc = f'{Data.inp_dir}/.run3/{Data.job}/*.root'
    l_path  = glob.glob(path_wc)

    npath   = len(l_path)
    if npath == 0:
        log.error(f'No file found in: {path_wc}')
        raise FileNotFoundError

    log.info(f'Found {npath} paths')

    return l_path
# ---------------------------------
def _split_paths(l_path):
    '''
    Takes list of paths to ROOT files
    Splits them into categories and returns a dictionary:

    category : [path_1, path_2, ...]
    '''
    npath = len(l_path)
    log.info(f'Splitting {npath} paths into categories')

    d_info_path = {}
    for path in tqdm.tqdm(l_path, ascii=' -'):
        info = _info_from_path(path)
        if info not in d_info_path:
            d_info_path[info] = []

        d_info_path[info].append(path)

    d_info_path = _truncate_paths(d_info_path)

    log.info('Found samples:')
    d_info_path = dict(sorted(d_info_path.items()))
    for info in d_info_path:
        log.info(info)

    return d_info_path
# ---------------------------------
def _truncate_paths(d_path):
    '''
    Will limit the number of paths in the values if Data.Max is larger than zero
    '''

    if Data.Max < 0:
        return d_path

    log.warning(f'Truncating to {Data.Max} paths')

    d_path_trunc = { key : val[:Data.Max] for key, val in d_path.items() }

    return d_path_trunc
# ---------------------------------
def _info_from_path(path):
    '''
    Will pick a path to a ROOT file
    Will return tuple with information associated to file
    This is needed to name output file and directories
    '''

    name = os.path.basename(path)
    if   name.startswith('dt_') or name.startswith('data_'):
        info = _info_from_data_path(path)
    elif name.startswith('mc_'):
        info = _info_from_mc_path(path)
    else:
        log.error(f'File name is not for data or MC: {name}')
        raise ValueError

    return info
# ---------------------------------
def _info_from_mc_path(path):
    '''
    Will return information from path to file
    '''
    name = os.path.basename(path)
    mtch = re.match(Data.mc_rgx, name)
    if not mtch:
        log.error(f'Cannot extract information from MC file: {name} using {Data.mc_rgx}')
        raise ValueError

    [evt_type, line] = mtch.groups()
    evt_type = int(evt_type)

    d_proc_evt = _get_proc_evt()
    if evt_type not in d_proc_evt:
        log.error(f'Event type {evt_type} not found, in:')
        pprint.pprint(d_proc_evt)

        raise ValueError

    proc = d_proc_evt[evt_type]

    #TODO: Do not hardcode year
    return proc, line, 'ana', '2024'
# ---------------------------------
def _info_from_data_path(path):
    '''
    Will get info from data path
    '''
    name = os.path.basename(path)
    mtc  = re.match(Data.dt_rgx, name)
    if not mtc:
        log.error(f'Cannot find kind in {name} using {Data.dt_rgx}')
        raise ValueError

    try:
        [year, decay] = mtc.groups()
    except ValueError as exc:
        log.error(f'Expected three elements in: {mtc.groups()}')
        raise ValueError from exc

    if len(year) == 2:
        year = f'20{year}'

    if 'MuMu' in decay:
        chan = 'mm'
    elif 'EE' in decay:
        chan = 'ee'
    else:
        log.error(f'Cannot find channel in {decay}')
        raise ValueError

    kind = _kind_from_decay(decay)

    return 'data', chan, kind, year
# ---------------------------------
@cache
def _get_proc_evt():
    '''
    Will load and return dictionary containing
    {event_type : process}
    '''

    file_path = files('post_ap_data').joinpath('link_conf.yaml')
    file_path = str(file_path)
    log.debug(f'Loading config from: {file_path}')
    if not os.path.isfile(file_path):
        log.error(f'YAML file with event type process correspondence not found: {file_path}')
        raise FileNotFoundError

    with open(file_path, encoding='utf-8') as ifile:
        d_cfg = yaml.safe_load(ifile)

    d_evt_proc = d_cfg['evt_proc']

    return d_evt_proc
# ---------------------------------
def _kind_from_decay(decay):
    '''
    Will take string symbolizing decay
    Will return kind of sample associated, e.g. analysis, calibration, same sign...
    '''

    if decay not in Data.cfg_dat['decays']:
        raise ValueError(f'Unrecognized decay: {decay}')

    return Data.cfg_dat['decays'][decay]
# ---------------------------------
def _load_config():
    conf_path = files('post_ap_data').joinpath('link_conf.yaml')
    conf_path = str(conf_path)
    if not os.path.isfile(conf_path):
        raise FileNotFoundError(f'Cannot find {conf_path}')

    with open(conf_path, encoding='utf-8') as ifile:
        Data.cfg_dat = yaml.safe_load(ifile)
# ---------------------------------
def _link_paths(info : tuple[str], l_path : list[str]) -> Union[str, None]:
    '''
    Makes symbolic links of list of paths of a specific kind
    info is a tuple with = (sample, channel, kind, year) information
    Will return directory where linked files are
    '''
    npath = len(l_path)
    log.info(f'Linking {npath} paths {info}')

    sam, chan, kind, year = info

    target_dir  = f'{Data.inp_dir}/{sam}_{chan}_{kind}/{Data.ver}/{year}'
    if os.path.isfile(f'{target_dir}.root'):
        log.warning(f'Merged file exists, not linking: {target_dir}.root')
        _save_summary(f'{target_dir}.root')
        return None

    os.makedirs(target_dir, exist_ok=True)
    log.info(f'Linking to: {target_dir}')
    if Data.dry:
        log.warning('Dry run, not linking')
        return None

    for source_path in tqdm.tqdm(l_path, ascii=' -'):
        name = os.path.basename(source_path)
        target_path = f'{target_dir}/{name}'

        log.debug(f'{source_path:<50}{"->":10}{target_path:<50}')
        _do_link_paths(src=source_path, tgt=target_path)

    return target_dir
# ---------------------------------
def _do_link_paths(src : str | None = None, tgt : str | None = None):
    '''
    Will check if target link exists, will delete it if it does
    Will make link
    '''
    if os.path.exists(tgt):
        os.unlink(tgt)

    os.symlink(src, tgt)
# ---------------------------------
def _merge_paths(target, l_path):
    '''
    Merge ROOT files of a specific kind
    '''
    npath = len(l_path)
    log.info(f'Merging {npath} paths {target}')

    if Data.dry:
        log.warning('Dry run, not merging')
        return

    log.info('')

    # Allow trees to got up to 1Tb when merging
    # https://root-forum.cern.ch/t/root-6-04-14-hadd-100gb-and-rootlogon/24581/3
    TTree.SetMaxTreeSize(1000_000_000_000)
    fm = TFileMerger(isLocal=True)
    for path in tqdm.tqdm(l_path, ascii=' -'):
        fm.AddFile(path, cpProgress=False)

    fm.OutputFile(target)
    success = fm.Merge()

    if not success:
        raise ValueError(f'Could not create merged file: {target}')

    _delete_tmp_files()
# ---------------------------------
def _delete_tmp_files():
    '''
    Delete files in /tmp that TFileMerger makes
    In order to prevent storage to run out
    '''

    log.info('Removing temporary files')

    l_path   = glob.glob('/tmp/ROOTMERGE*.root')
    nremoved = 0
    for path in l_path:
        try:
            os.remove(path)
        except:
            pass

        nremoved += 1

    log.info(f'Removed {nremoved} files')
# ---------------------------------
def _save_summary(target):
    '''
    Make text file with summary of file, e.g. 2024.root -> 2024.txt
    '''
    if Data.dry:
        return

    prt = RFPrinter(path=target)
    prt.save()
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    _load_config()

    l_path = _get_paths()
    d_path = _split_paths(l_path)
    for kind, l_path in d_path.items():
        target_dir = _link_paths(kind, l_path)
        if target_dir is None:
            continue

        target = f'{target_dir}.root'
        _merge_paths(target, l_path)
        _save_summary(target)
# ---------------------------------
if __name__ == '__main__':
    main()
