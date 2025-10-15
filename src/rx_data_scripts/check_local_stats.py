'''
Script used to print statistics on files stored in cluster
'''
import os
import glob
import math
import argparse
from pathlib  import Path
from colorama import Fore, Style

import pandas   as pnd
from ap_utilities.decays   import utilities          as aput
from dmu.generic           import version_management as vmn
from dmu.logging.log_store import LogStore
from dmu.generic           import typing_utilities   as tut
from rx_data               import utilities          as dut
from rx_common             import info

log=LogStore.add_logger('rx_data:check_local_stats')
# --------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    project : str
# --------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to print statistics on files stored in cluster')
    parser.add_argument('-p', '--project', type=str, help='Path to file storing lists of samples', required=True)
    args = parser.parse_args()

    Data.project = args.project
# --------------------------------------
def _get_friend_stats(frnd_dir : str, kind : str) -> dict[str,int]:
    '''
    Parameters
    --------------------
    frnd_dir: directory with friend tree files
    kind    : Type of quantity requested for these trees

    Returns
    --------------------
    dictionary where:

    key  : Name of sample
    value: Quantity requested 
    '''
    l_fpath = _paths_from_friend_dir(frnd_dir=frnd_dir)
    d_sample= {}
    for fpath in l_fpath:
        sample, trigger = dut.info_from_path(path=fpath)
        sample          = aput.name_from_lower_case(sample)
        chan            = info.channel_from_trigger(trigger=trigger)

        if not sample.startswith('DATA'):
            event_type= aput.read_event_type(nickname=sample)
            identifier = f'{chan}/{sample}/{event_type}'
        else:
            identifier = f'{chan}/{sample}'

        if identifier not in d_sample:
            d_sample[identifier] = _stat_from_path(fpath=fpath, kind=kind)
        else:
            d_sample[identifier]+= _stat_from_path(fpath=fpath, kind=kind) 

    return d_sample
# ----------------------
def _paths_from_friend_dir(frnd_dir : str) -> list[str]:
    '''
    Parameters
    -------------
    frnd_dir: Directory with friend trees

    Returns
    -------------
    List of ROOT files
    '''
    fpath_wc = f'{frnd_dir}/*.root'
    vers_dir = vmn.get_last_version(frnd_dir, version_only=False)

    fpath_wc = f'{vers_dir}/*.root'
    l_fpath  = glob.glob(fpath_wc)
    if len(l_fpath) == 0:
        raise ValueError(f'No file found in {fpath_wc}')

    return l_fpath
# ----------------------
def _stat_from_path(fpath : str, kind : str) -> int:
    '''
    Parameters
    -------------
    fpath : Path to file
    kind  : Property of path

    Returns
    -------------
    Size in Mb
    '''
    if kind == 'number':
        return 1

    if kind == 'size':
        path = Path(fpath)
        size = path.stat().st_size / 1_000_000

        return int(size)

    raise ValueError(f'Invalid property: {kind}')
# --------------------------------------
def _get_df() -> pnd.DataFrame:
    '''
    Returns dataframe with friend trees as columns
    Samples as the index and the number of files in the cells
    '''
    ana_dir   = os.environ['ANADIR']
    root_path = f'{ana_dir}/Data/{Data.project}'
    l_frnd_dir= glob.glob(f'{root_path}/*')

    data = {}
    for frnd_dir in l_frnd_dir:
        frnd = os.path.basename(frnd_dir)
        data[frnd] = _get_friend_stats(frnd_dir=frnd_dir, kind='number')

    for frnd_dir in l_frnd_dir:
        frnd = os.path.basename(frnd_dir)
        size = _get_friend_stats(frnd_dir=frnd_dir, kind='size')
        data = _update_sizes(data=data, sizes=size)

    df = pnd.DataFrame.from_dict(data, orient='columns')
    df = df.rename(columns={'Sizes' : 'Size [Mb]'})

    return df
# ----------------------
def _update_sizes(data : dict[str,dict[str,int]], sizes : dict[str,int]) -> dict[str,dict[str,int]]:
    '''
    Parameters
    -------------
    data: Dictionary where:
        Key  : Friend trees names 
        Value: Dictionary with:
            key as Sample name and value as number of files
            key as Sample size and value as size in Mb

    sizes: Dictionary with:
        Key  : Sample nickname
        value: main tree size

    Returns
    -------------
    data dictionary with updated sizes
    '''
    data['Sizes'] = {}
    for identifier, size in sizes.items():
        data['Sizes'][identifier] = size 
        chan = identifier.split('/')[0]
        if chan == 'MM':
            continue

        if 'identifier' not in data['brem_track_2']:
            data['brem_track_2'][identifier] = 0

    return data
# --------------------------------------
def _colorize(row : pnd.Series) -> pnd.Series:
    '''
    Parameters
    -------------
    row: Pandas series with statistics

    Returns
    -------------
    Series with colors
    '''
    try:
        expected = row['main']
    except KeyError:
        raise KeyError(f'Cannot get main from: {row}')

    excluded = {'Size [Mb]', 'main'}
    colored  = row.copy()

    for col in row.index:
        if col in excluded:
            continue

        val = tut.numeric_from_series(row, col, float)
        if not math.isnan(val) and val != expected:
            colored[col] = f'{Fore.RED}{val}{Style.RESET_ALL}'

    return colored
# --------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    df = _get_df()
    df = df.apply(_colorize, axis=1)
    df = df.sort_index()

    pnd.set_option('display.max_rows'    , None)
    pnd.set_option('display.max_columns' ,   10)
    pnd.set_option('display.width'       , None)
    pnd.set_option('display.max_colwidth', None)

    print(df.to_markdown())
# --------------------------------------
if __name__ == '__main__':
    main()
