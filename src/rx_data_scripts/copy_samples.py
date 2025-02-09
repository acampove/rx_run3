'''
Script used to copy ntuples from mounted filesystem
'''
import os
import shutil
import argparse

import yaml
import ap_utilities.decays.utilities as aput
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:copy_samples')
# -----------------------------------------
class Data:
    '''
    Class holding attributes meant to be shared
    '''
    path     : str
    samp     : str
    out_dir  : str

    d_config : dict
    d_data   : dict
# -----------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to copy files from remote server to laptop')
    parser.add_argument('-p', '--path', type=str, help='Path to YAML file with paths samples existing in source file system')
    parser.add_argument('-s', '--samp', type=str, help='Path to YAML files with samples to be copied')
    args = parser.parse_args()

    Data.path = args.path
    Data.samp = args.samp
# -----------------------------------------
def _get_samples_to_copy() -> list[str]:
    d_samp = Data.d_config['samples']
    l_evt = []
    for kind in d_samp:
        l_evt+= d_samp[kind]

    l_name = [ aput.read_decay_name(event_type=str(event_type)) for event_type in l_evt ]

    nfile = len(l_name)
    log.info(f'Will copy {nfile} samples')

    return l_name
# -----------------------------------------
def _initialize():
    with open(Data.samp, encoding='utf-8') as ifile:
        d_config = yaml.safe_load(ifile)

    Data.out_dir = d_config['out_dir']
    os.makedirs(Data.out_dir, exist_ok=True)
    log.info(f'Copying files to: {Data.out_dir}')

    Data.d_config = d_config

    with open(Data.path, encoding='utf-8') as ifile:
        Data.d_data = yaml.safe_load(ifile)
# -----------------------------------------
def _copy_sample(sample : str) -> None:
    if sample not in Data.d_data:
        raise ValueError(f'Cannot find {sample}')

    for trigger in Data.d_data[sample]:
        l_path = Data.d_data[sample][trigger]
        npath  = len(l_path)

        log.info(f'Copying {npath} files from sample {sample}')

        for source in l_path:
            fname = os.path.basename(source)
            target= f'{Data.out_dir}/{fname}'
            shutil.copy(source, target)
# -----------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    _initialize()

    l_to_copy = _get_samples_to_copy()
    for sample in l_to_copy:
        _copy_sample(sample)
# -----------------------------------------
if __name__ == '__main__':
    main()
