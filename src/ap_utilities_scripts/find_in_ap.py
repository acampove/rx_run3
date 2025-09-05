'''
This script is meant to:

- Search for all the lines in:

$ANADIR/bkk_checker/block_*/info.yaml

- Check if ntuples corresponding to each line exist in BKK.
- Build a new info.yaml for missing samples
'''
import os
import glob

import apd
import pandas as pnd

from apd                   import SampleCollection
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ap_utilities:find_in_ap')
# ----------------------
def _info_from_line(line : str) -> tuple[str, str, str]:
    '''
    Parameters
    -------------
    line: Line meant to go in info.yaml

    Returns
    -------------
    Tuple with:

    Event type
    Block, e.g w32_32
    Line meant to be added to info.yaml
    '''
    line    = line.replace('(', '').replace(')', '')
    line    = line.replace('"', '').replace('"', '')
    l_field = line.split(',')
    evt_type= l_field[1].replace(' ', '')
    block   = l_field[2].replace('2024.', '').replace('.', '_').lower().replace(' ', '')

    return evt_type, block, line
# ----------------------
def _get_info() -> list[tuple[str, str, str]]:
    '''
    Returns
    -------------
    Dictionary with:

    Key  : EventType
    Value: Line that would go in info.yaml
    '''
    ana_dir = os.environ['ANADIR']
    path_wc = f'{ana_dir}/bkk_checker/block_*/info.yaml'
    l_path  = glob.glob(path_wc)
    l_path  = sorted(l_path)

    l_line  = []
    log.info('Reading files')
    for path in l_path:
        log.debug(f'    {path}')
        with open(path) as ifile:
            l_line += ifile.read().splitlines()

    return [ _info_from_line(line=line) for line in l_line ]
# ----------------------
def _found_type(
    col      : SampleCollection,
    evt_type : str, 
    block    : str) -> bool:
    '''
    Parameters
    -------------
    col     : Instance of SampleCollection
    evt_type: Event type
    block   : E.g. w40_42

    Returns
    -------------
    True if already found in AP
    '''
    rep  = col.filter(eventtype=evt_type).report()
    df   = pnd.DataFrame(rep[1:], columns=rep[0])
    found= False

    if 'name' not in df.columns:
        raise ValueError(f'Cannot find sample name for: {evt_type}/{block}')

    for name in df['name']:
        if block not in name:
            log.verbose(f'Block {block}({type(block)}) not in {name}({type(name)})')
            continue

        if not name.endswith('_tuple'):
            log.verbose(f'{name} does not end in _tuple')
            continue

        if '_spr,' in name:
            log.verbose(f'{name} contains _spr,')
            continue

        found = True
        break

    if not found:
        log.debug(f'Cannot find: {evt_type}/{block}')
    else:
        log.debug(f'Found: {evt_type}/{block}')

    return found 
# ----------------------
def main():
    '''
    Entry point
    '''
    LogStore.set_level('ap_utilities:find_in_ap', 20)

    dset      = apd.get_analysis_data(working_group='RD', analysis='rd_ap_2024')
    scol      = dset.all_samples()
    if not isinstance(scol, SampleCollection):
        raise RuntimeError('Cannot extract SampleCollection instance')

    t_info    = _get_info()
    l_missing = [ line for etype, block, line in t_info if not _found_type(col=scol, evt_type=etype, block=block) ]
    l_missing = sorted(l_missing)

    total     = len(t_info)
    missing   = len(l_missing)

    log.info(f'Missing: {missing}/{total}')
    with open('info.yaml', 'w') as ofile:
        for line in l_missing:
            ofile.write(f'{line}\n')
# ----------------------
if __name__ == '__main__':
    main()
