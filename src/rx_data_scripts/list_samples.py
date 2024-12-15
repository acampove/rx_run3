'''
Script used to show a summary of samples
'''
import os
import re
import json
import glob
import argparse
from importlib.resources    import files
from dataclasses            import dataclass

from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:list_samples')

# pylint: disable=line-too-long
# ----------------------------
@dataclass
class Data:
    '''
    Data class storing shared attributes
    '''

    version : str
    dt_rgx  = r'(data_\d{2}_.*)_(\w+RD_.*)_\w{10}\.root'
    mc_rgx  = r'mc_.*_\d{8}_(.*)_(\w+RD_.*)_\w{10}\.root'
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used list samples for a given version')
    parser.add_argument('-v', '--vers' , type=str, help='Version of LFNs', required=True)

    args = parser.parse_args()
    Data.version = args.vers
# ----------------------------
def _sample_from_lfn(lfn : str) -> str:
    file_name = os.path.basename(lfn)

    mtch_mc = re.match(Data.mc_rgx, file_name)
    mtch_dt = re.match(Data.dt_rgx, file_name)

    if mtch_mc:
        return mtch_mc.group(1)

    if mtch_dt:
        return mtch_dt.group(1)

    raise ValueError(f'Cannot extract sample name from: {file_name}')
# ----------------------------
def _get_samples() -> dict[str,int]:
    lfn_wc = files('rx_data_lfns').joinpath(f'{Data.version}/*.json')
    lfn_wc = str(lfn_wc)
    l_path = glob.glob(lfn_wc)

    if len(l_path) == 0:
        raise ValueError(f'No files found in: {lfn_wc}')

    l_lfn  = []

    for path in l_path:
        with open(path, encoding='utf-8') as ifile:
            l_lfn += json.load(ifile)

    d_sample = {}
    for lfn in l_lfn:
        sample = _sample_from_lfn(lfn)
        if sample not in d_sample:
            d_sample[sample] = 1
        else:
            d_sample[sample] =+1

    return d_sample
# ----------------------------
def main():
    '''
    Starts here
    '''

    _parse_args()
    d_sample = _get_samples()

    log.info(60 * '-')
    log.info(f'{"Sample":<50}{"Files":<10}')
    log.info(60 * '-')
    for name, nfile in d_sample.items():
        log.info(f'{name:<50}{nfile:<10}')
# ----------------------------
if __name__ == '__main__':
    main()
