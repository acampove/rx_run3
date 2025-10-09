'''
Script used to merge ROOT files
'''

import os
import glob
import argparse
import subprocess
from pathlib import Path

from ROOT                  import TFileMerger # type: ignore
from dmu.logging.log_store import LogStore
from dmu.generic           import version_management as vmn
from rx_data.path_splitter import PathSplitter

log = LogStore.add_logger('rx_data:merge_samples')
# --------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    dry_run      : bool
    samples_path : str|Path
    proj         : str
    vers         : str|None
    sample_name  : str
    trigger_name : str
    out_dir      : str
# --------------------------------------
def _initialize():
    ana_dir= os.environ['ANADIR']
    sam_dir= f'{ana_dir}/Data/{Data.proj}/main'
    if Data.vers is None:
        sam_dir = vmn.get_last_version(dir_path=sam_dir, version_only=False)
    else:
        sam_dir = f'{sam_dir}/{Data.vers}'

    Data.samples_path = sam_dir
    Data.out_dir      = _get_out_dir()
# --------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to merge ROOT files from list of samples')
    parser.add_argument('-p', '--proj' , type=str, help='Project associated to samples, e.g. rx'            , required=True)
    parser.add_argument('-s', '--samp' , type=str, help='Name of sample to merge, e.g. DATA_24_MagDown_24c3', required=True)
    parser.add_argument('-t', '--trig' , type=str, help='Trigger, e.g Hlt2RD_BuToKpEE_MVA'                  , required=True)
    parser.add_argument('-v', '--vers' , type=str, help='Version of ntuples, default latest')
    parser.add_argument('-l', '--logl' , type=int, help='Logging level', default = 20)
    parser.add_argument('-d', '--dryr' ,           help='If used, will do dry run'              , action='store_true')
    args = parser.parse_args()

    Data.proj         = args.proj
    Data.dry_run      = args.dryr
    Data.sample_name  = args.samp
    Data.trigger_name = args.trig
    Data.vers         = args.vers

    LogStore.set_level('rx_data:merge_samples', args.logl)
    LogStore.set_level('rx_data:path_splitter', args.logl)
# --------------------------------------
def _get_out_dir() -> str:
    config_dir = os.path.dirname(Data.samples_path)
    out_dir    = f'{config_dir}/merged'

    os.makedirs(out_dir, exist_ok=True)

    return out_dir
# --------------------------------------
def _get_paths() -> list[str]:
    '''
    Returns paths to ROOT files for a given sampe
    '''
    path_wc= f'{Data.samples_path}/*.root'
    l_path = glob.glob(path_wc)
    npath  = len(l_path)
    if npath == 0:
        raise ValueError(f'Found no files in {path_wc}')

    if not l_path:
        raise ValueError(f'No paths found to split in: {path_wc}')

    return l_path
# --------------------------------------
def _get_samples() -> dict:
    l_path = _get_paths()

    log.debug(f'Will split {len(l_path)} paths')
    splt   = PathSplitter(paths=l_path)
    d_path = splt.split(nested=True)

    if not d_path:
        raise ValueError('No paths found after splitting')

    return d_path
# ----------------------------
def _merge_paths(l_path : list[str]) -> None:
    sample_name = Data.sample_name.lower()
    out_path    = f'{Data.out_dir}/{sample_name}_{Data.trigger_name}.root'
    if os.path.isfile(out_path):
        log.info(f'File already found: {out_path}')
        return

    npath = len(l_path)
    log.info(f'Merging {npath} paths for {sample_name}/{Data.trigger_name}')

    fm = TFileMerger(isLocal=False)
    fm.SetFastMethod(True)
    for path in l_path:
        fm.AddFile(path, cpProgress=False)

    fm.OutputFile(out_path, 'RECREATE')
    if Data.dry_run:
        log.warning('Skipping merging step for dry run')
        return

    success = fm.Merge()
    if not success:
        raise RuntimeError('Merge failed')

    _remove_objects(out_path)
# ----------------------------
def _remove_objects(out_path : str) -> None:
    log.info('Removing not needed objects from merged file')
    result = subprocess.run(['rootrm', f'{out_path}:metadata'], capture_output=True, text=True, check=False)

    if result.returncode != 0:
        raise RuntimeError('Cannot delete metadata from file')
# ----------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    _initialize()

    d_data = _get_samples()
    if Data.sample_name not in d_data:
        for sample in d_data:
            log.info(sample)
        raise ValueError(f'Sample {Data.sample_name} not found')

    if Data.trigger_name not in d_data[Data.sample_name]:
        for trigger in d_data[Data.sample_name]:
            log.info(trigger)
        raise ValueError(f'Trigger {Data.trigger_name} not found for sample {Data.sample_name}')

    l_path = d_data[Data.sample_name][Data.trigger_name]

    _merge_paths(l_path)

    log.info('Merge finished')
# ----------------------------
if __name__ == '__main__':
    main()
