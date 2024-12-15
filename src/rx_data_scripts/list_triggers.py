'''
Script used to show a summary of triggers
'''
import os
import re
import json
import glob
import argparse
from importlib.resources    import files
from dataclasses            import dataclass

import yaml
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:list_triggers')

# pylint: disable=line-too-long
# ----------------------------
@dataclass
class Data:
    '''
    Data class storing shared attributes
    '''

    version : str
    outfile : str
    dt_rgx  = r'(data_\d{2}_.*)_(\w+RD_.*)_\w{10}\.root'
    mc_rgx  = r'mc_.*_\d{8}_(.*)_(\w+RD_.*)_\w{10}\.root'
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used list triggers for a given version')
    parser.add_argument('-v', '--vers' , type=str, help='Version of LFNs', required=True)
    parser.add_argument('-o', '--outf' , type=str, help='Name of file to save list as YAML, by default will not save anything')

    args = parser.parse_args()
    Data.version = args.vers
    Data.outfile = args.outf
# ----------------------------
def _trigger_from_lfn(lfn : str) -> str:
    file_name = os.path.basename(lfn)

    mtch_mc = re.match(Data.mc_rgx, file_name)
    mtch_dt = re.match(Data.dt_rgx, file_name)

    if mtch_mc:
        return mtch_mc.group(2)

    if mtch_dt:
        return mtch_dt.group(2)

    raise ValueError(f'Cannot extract trigger name from: {file_name}')
# ----------------------------
def _get_triggers() -> dict[str,int]:
    lfn_wc = files('rx_data_lfns').joinpath(f'{Data.version}/*.json')
    lfn_wc = str(lfn_wc)
    l_path = glob.glob(lfn_wc)

    if len(l_path) == 0:
        raise ValueError(f'No files found in: {lfn_wc}')

    l_lfn  = []

    for path in l_path:
        with open(path, encoding='utf-8') as ifile:
            l_lfn += json.load(ifile)

    nlfn = len(l_lfn)
    log.info(f'Found {nlfn} LFNs')

    d_trigger = {}
    for lfn in l_lfn:
        trigger = _trigger_from_lfn(lfn)
        if trigger not in d_trigger:
            d_trigger[trigger] = 1
        else:
            d_trigger[trigger]+= 1

    return d_trigger
# ----------------------------
def _save(d_trigger : dict[str,int]) -> None:
    if not hasattr(Data, 'outfile'):
        return

    with open(Data.outfile, 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_trigger, ofile)
# ----------------------------
def main():
    '''
    Starts here
    '''

    _parse_args()
    d_trigger = _get_triggers()

    _save(d_trigger)

    log.info(60 * '-')
    log.info(f'{"trigger":<50}{"Files":<10}')
    log.info(60 * '-')
    for name, nfile in d_trigger.items():
        log.info(f'{name:<50}{nfile:<10}')
# ----------------------------
if __name__ == '__main__':
    main()
