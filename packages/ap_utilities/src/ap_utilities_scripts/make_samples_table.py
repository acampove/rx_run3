'''
This script is used to make a latex table from the list of 
MC samples in:

ap_utilities_data/analyses/by_priority.yaml
'''

import argparse
import pandas as pnd

from dmu.pdataframe        import utilities as put
from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut

log=LogStore.add_logger('ap_utilities:make_samples_table')
# ----------------------
def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Script used to make latex tables with MC sample information')
    parser.add_argument('-c', '--config' , type=str, help='Name of file with MC samples information')
    args = parser.parse_args()

    return args
# ----------------------
def _get_df(name : str) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    name: Name of config file with samples

    Returns
    -------------
    Pandas dataframe with information
    '''
    evt_type = []
    decays   = []
    kind     = []

    cfg = gut.load_conf(package='ap_utilities_data', fpath=f'analyses/{name}.yaml')
    for section, data in cfg.items():
        if section == 'all':
            continue

        if section == 'run12_rx':
            section = 'Dropped'

        evt_type += list(data.keys())
        decays   += list(data.values())
        kind     += [section] * len(data)

    df  = pnd.DataFrame({'Decays' : decays, 'Event Type' : evt_type, 'Kind' : kind})

    return df
# ----------------------
def main():
    '''
    Entry point
    '''
    args = _parse_args()
    df   = _get_df(name=args.config)

    put.df_to_tex(df,
            './table.tex',
            caption  = 'some caption')
# ----------------------
if __name__ == '__main__':
    main()
