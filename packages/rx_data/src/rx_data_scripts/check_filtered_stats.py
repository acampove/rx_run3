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

from rx_data.filtered_stats import FilteredStats
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_data:check_filtered_stats')
# ----------------------
def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Script used to check statistics from filtered ntuples in the grid')
    parser.add_argument('-v', '--versions' , nargs='+', type=int, help='Versions to use', required=True)
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
    fst = FilteredStats(analysis='rx', versions=cfg.versions)
    df  = fst.get_df()

    return df
# ----------------------
def main():
    '''
    Entry point
    '''
    pnd.set_option('display.max_columns' , None)
    pnd.set_option('display.max_colwidth', None)
    pnd.set_option('display.width'       ,    0)

    cfg = _parse_args()
    df  = _get_df(cfg)

    df  = df[df['Sample'] != 'data'   ]
    df  = df[df['block' ] != 'missing']

    for block, df_block in df.groupby('block'):
        log.info('')
        log.info(f'Block: {block}')
        df_block = df_block.drop(columns=['block'])

        for mag, df_mag in df_block.groupby('Mag'):
            log.info(f'Mag: {mag}')
            df_mag = df_mag.drop(columns=['Mag'])
            df_mag = df_mag.reset_index(drop=True)

            df_mag = df_mag.sort_values(by='Version')
            print(df_mag)
# ----------------------
if __name__ == '__main__':
    main()
