'''
Script used to show a summary of triggers
'''
# pylint: disable=line-too-long, import-error

import json
import glob
import argparse

from typing                 import Union
from dataclasses            import dataclass
from importlib.resources    import files

import yaml
from dmu.logging.log_store  import LogStore
from rx_data                import utilities as ut

log = LogStore.add_logger('rx_data:list_triggers')

# ----------------------------
@dataclass
class Data:
    '''
    Data class storing shared attributes
    '''

    version : str
    kind    : str
    outfile : Union[str,None]
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used list triggers for a given version')
    parser.add_argument('-v', '--vers' , type=str, help='Version of LFNs', required=True)
    parser.add_argument('-k', '--kind' , type=str, help='Type of production', choices=['rx', 'lbpkmumu'], required=True)
    parser.add_argument('-o', '--outf' , type=str, help='Name of file to save list as YAML, by default will not save anything')
    parser.add_argument('-l', '--level', type=int, help='Logging level', default=20)

    args = parser.parse_args()
    Data.version = args.vers
    Data.outfile = args.outf
    Data.kind    = args.kind

    LogStore.set_level('rx_data:list_triggers', args.level)
# ----------------------------
def _get_triggers() -> dict[str,int]:
    lfn_wc = files('rx_data_lfns').joinpath(f'{Data.kind}/{Data.version}/*.json')
    lfn_wc = str(lfn_wc)
    l_path = glob.glob(lfn_wc)

    npath  = len(l_path)
    log.debug(f'Found {npath} paths in {lfn_wc}')

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
        _, trigger = ut.info_from_path(lfn)
        if trigger not in d_trigger:
            d_trigger[trigger] = 1
        else:
            d_trigger[trigger]+= 1

    d_trigger_sorted = { name : d_trigger[name] for name in sorted(d_trigger) }

    return d_trigger_sorted
# ----------------------------
def _save(d_trigger : dict[str,int]) -> None:
    if Data.outfile is None:
        return

    log.info(f'Saving trigger list to: {Data.outfile}')
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
