'''
Script used to print statistics on files stored in cluster
'''
import os
import glob
import math
import argparse
from pathlib import Path

import pandas   as pnd
from ap_utilities.decays   import utilities          as aput
from dmu.generic           import version_management as vmn
from dmu.logging.log_store import LogStore
from rx_data               import utilities          as dut

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
    directory with friend tree files

    Returns
    --------------------
    dictionary where:

    key  : Name of sample
    value: Number of files
    '''
    l_fpath = _paths_from_friend_dir(frnd_dir=frnd_dir)
    d_sample= {}
    for fpath in l_fpath:
        sample, _ = dut.info_from_path(path=fpath)
        sample    = aput.name_from_lower_case(sample)
        if sample not in d_sample:
            d_sample[sample] = _stat_from_path(fpath=fpath, kind=kind)
        else:
            d_sample[sample]+= _stat_from_path(fpath=fpath, kind=kind) 

    return d_sample
    fpath_wc = f'{frnd_dir}/*.root'
    vers_dir = vmn.get_last_version(frnd_dir, version_only=False)

    fpath_wc = f'{vers_dir}/*.root'
    l_fpath  = glob.glob(fpath_wc)
    if len(l_fpath) == 0:
        raise ValueError(f'No file found in {fpath_wc}')


    return d_sample
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
        data[frnd] = _get_friend_stats(frnd_dir=frnd_dir)

    df = pnd.DataFrame.from_dict(data, orient='columns')

    return df
# --------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    df = _get_df()
    df = df.sort_index()

    pnd.set_option('display.max_rows'    , None)
    pnd.set_option('display.max_columns' ,   10)
    pnd.set_option('display.width'       , None)
    pnd.set_option('display.max_colwidth', None)

    print(df.to_markdown())
# --------------------------------------
if __name__ == '__main__':
    main()
