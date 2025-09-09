'''
This script will:

- Open the JSON files with the lists of LFNs of filtered files
- Open the corresponding files in the GangaSandbox
- Match the samples in the former with the latter
- Build a table in the form of a dataframe showing:
    - Event type
    - Sample name
    - Block number
'''

import argparse
import pandas as pnd
from pathlib import Path

from rx_data.filtered_stats import FilteredStats
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_data:check_filtered_stats')
# ----------------------
def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Script used to check statistics from filtered ntuples in the grid')
    parser.add_argument('-m', '--min_vers' , type=int, help='Minimum version from which to start check, e.g 5 (v5)', required=True)
    args = parser.parse_args()

    return args
# ----------------------
def _get_df(cfg : argparse.Namespace) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    cfg: Argparse object with options

    Returns
    -------------
    Pandas dataframe with information
    '''
    cache_dir = Path('.cache/')
    cache_dir.mkdir(exist_ok=True)
    out_path = cache_dir/f'data_{cfg.min_vers}.parquet'
    if out_path.is_file():
        log.info(f'Loading from: {out_path}')
        return pnd.read_parquet(out_path)

    fst = FilteredStats(analysis='rx', min_vers=cfg.min_vers)
    df  = fst.get_df()

    df.to_parquet(out_path)
    log.info(f'Caching to: {out_path}')

    return df
# ----------------------
def main():
    '''
    Entry point
    '''
    cfg = _parse_args()
    df  = _get_df(cfg)
# ----------------------
if __name__ == '__main__':
    main()
