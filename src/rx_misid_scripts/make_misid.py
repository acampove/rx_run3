'''
Script intended to create pandas dataframes with weighted misID control region
'''
import os
import copy
import argparse
import multiprocessing
from importlib.resources import files

import yaml
import pandas as pnd
from dmu.logging.log_store     import LogStore
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:make_misid')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    sample    : str
    q2bin     : str
    version   : str

    out_dir   = 'misid_output'
    log_lvl   : int
    cfg       : dict
# -------------------------------------------------------
def _set_log():
    LogStore.set_level('rx_misid:make_misid'      , Data.log_lvl)
    LogStore.set_level('rx_misid:misid_calculator', Data.log_lvl)
    LogStore.set_level('rx_misid:splitter'        , Data.log_lvl)
    LogStore.set_level('rx_misid:weighter'        , Data.log_lvl)
# ---------------------------------
def _get_config() -> dict:
    config_path = files('rx_misid_data').joinpath(f'misid_{Data.version}.yaml')
    with open(config_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    cfg['input']['q2bin' ] = Data.q2bin

    return cfg
# ---------------------------------
def _initialize() -> None:
    _set_log()
    Data.cfg = _get_config()
    os.makedirs(Data.out_dir, exist_ok=True)
# ---------------------------------
def _get_samples() -> list[str]:
    if Data.sample == 'data':
        l_dset = ['24c1', '24c2', '24c3', '24c4']
        l_magn = ['MagUp', 'MagDown']
        return [ f'DATA_24_{magn}_{dset}' for magn in l_magn for dset in l_dset ]

    if Data.sample == 'signal':
        return ['Bu_Kee_eq_btosllball05_DPC']

    if Data.sample == 'leakage':
        return ['Bu_JpsiK_ee_eq_DPC']

    raise ValueError(f'Invalid sample: {Data.sample}')
# ---------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script needed to calculate pandas dataframes with weighted misID control regions')
    parser.add_argument('-s', '--sample' , type=str, help='Sample name', required=True, choices=['data', 'signal' , 'leakage'])
    parser.add_argument('-q', '--q2bin'  , type=str, help='Q2 bin'     , required=True, choices=['low' , 'central', 'high'   ])
    parser.add_argument('-v', '--version', type=str, help='Version'    , required=True)
    parser.add_argument('-l', '--log_lvl', type=int, help='Logging level', default=20, choices=[10, 20, 30])
    args = parser.parse_args()

    Data.sample = args.sample
    Data.q2bin  = args.q2bin
    Data.version= args.version
    Data.log_lvl= args.log_lvl
# ---------------------------------
def _make_dataframe(sample : str) -> pnd.DataFrame:
    cfg = copy.deepcopy(Data.cfg)
    cfg['input']['sample'] = sample

    obj = MisIDCalculator(cfg=cfg)
    df  = obj.get_misid()

    return df
# ---------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _initialize()

    l_sample = _get_samples()
    nsample  = len(l_sample)

    if nsample == 1:
        df = _make_dataframe(l_sample[0])
    else:
        log.info(f'Using {nsample} processes')
        with multiprocessing.Pool(processes=nsample) as pool:
            l_df = pool.map(_make_dataframe, l_sample)

        df = pnd.concat(l_df, axis=0, ignore_index=True)

    out_path = f'{Data.out_dir}/{Data.sample}_{Data.q2bin}.parquet'
    log.info(f'Saving to: {out_path}')
    df.to_parquet(out_path)
# ---------------------------------
if __name__ == '__main__':
    main()
