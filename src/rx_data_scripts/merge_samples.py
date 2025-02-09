'''
Script used to merge ROOT files
'''

import os
import argparse
import yaml

import tqdm
from ROOT                  import TFileMerger, TFile
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_data:merge_samples')
# --------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    samples_path : str
    sample_name  : str
    trigger_name : str
    out_dir      : str
# --------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to merge ROOT files from list of samples')
    parser.add_argument('-p', '--path' , type=str, help='Path to file storing lists of samples', required=True)
    parser.add_argument('-s', '--samp' , type=str, help='Name of sample to merge', required=True)
    parser.add_argument('-t', '--trig' , type=str, help='Trigger'                , required=True)
    args = parser.parse_args()

    Data.samples_path = args.path
    Data.sample_name  = args.samp
    Data.trigger_name = args.trig
    Data.out_dir      = _get_out_dir()
# --------------------------------------
def _get_out_dir() -> str:
    config_dir = os.path.dirname(Data.samples_path)
    out_dir    = f'{config_dir}/merged'

    os.makedirs(out_dir, exist_ok=True)

    return out_dir
# --------------------------------------
def _get_samples() -> dict:
    with open(Data.samples_path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# ----------------------------
def _merge_paths(l_path : list[str]) -> None:
    out_path = f'{Data.out_dir}/{Data.sample_name}_{Data.trigger_name}.root'
    if os.path.isfile(out_path):
        _remove_objects(out_path)
        log.info(f'File already found: {out_path}')
        return

    npath = len(l_path)
    log.info(f'Mergin {npath} paths for {Data.sample_name}/{Data.trigger_name}')

    fm = TFileMerger(isLocal=True)
    for path in tqdm.tqdm(l_path, ascii=' -'):
        fm.AddFile(path, cpProgress=False)

    fm.OutputFile(out_path, 'RECREATE')
    success = fm.Merge()
    if not success:
        raise RuntimeError('Merge failed')

    _remove_objects(out_path)
# ----------------------------
def _remove_objects(out_path : str) -> None:
    log.info('Removing not needed objects from merged file')

    ifile = TFile.Open(out_path, 'update')
    l_key = ifile.GetListOfKeys()

    i_deleted = 0
    for key in l_key:
        obj_name   = key.GetName()
        class_name = key.ReadObj().GetClassName()

        if class_name != 'TObjString':
            continue

        i_deleted += 1
        ifile.Delete(obj_name)

    log.info(f'Deleted {i_deleted} objects')

    ifile.Close()
# ----------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    d_data = _get_samples()
    if Data.sample_name not in d_data:
        raise ValueError(f'Sample {Data.sample_name} not found')

    if Data.trigger_name not in d_data[Data.sample_name]:
        raise ValueError(f'Trigger {Data.trigger_name} not found for sample {Data.sample_name}')

    l_path = d_data[Data.sample_name][Data.trigger_name]

    _merge_paths(l_path)
# ----------------------------
if __name__ == '__main__':
    main()
