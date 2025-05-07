'''
Script used to transform JSON files with fitting parameters to create a dataframe
with the widths and means of the Jpsi peak, fitted from data and MC samples
'''
import os
import re
import glob
import math
import argparse
from typing import Callable

import numpy
import jacobi
import pandas                as pnd
import dmu.generic.utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_q2:dump_q2_ratios')
#-------------------------------------
class Data:
    '''
    Data class
    '''
    regex   = r'Hlt2RD_BuToKpEE_MVA_2024_(\d)_(\d)_nom'
    inp_dir : str
    version : str
    l_par   = ['rsg', 'dmu', 'mMC', 'mDT']
#-------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Used to create pandas dataframe with information from fits needed to smear q2')
    parser.add_argument('-v', '--vers', type=str, help='Version'        , required=True)
    args = parser.parse_args()

    Data.version = args.vers
#-------------------------------------
def _initialize():
    ana_dir      = os.environ['ANADIR']
    Data.inp_dir = f'{ana_dir}/q2/fits/{Data.version}'
#-------------------------------------
def _row_from_path(path : str) -> list[float]:
    data = gut.load_json(path)

    [[mu_val, mu_err]] = [ val for name, val in data.items() if name.startswith('mu_')]
    [[sg_val, sg_err]] = [ val for name, val in data.items() if name.startswith('sg_')]

    brem, block = _brem_block_from_path(path=path)

    return [mu_val, mu_err, sg_val, sg_err, brem, block]
#-------------------------------------
def _brem_block_from_path(path : str) -> tuple[str,str]:
    dir_name = os.path.dirname(path)
    sample   = os.path.basename(dir_name)

    mtch     = re.match(Data.regex, sample)
    if not mtch:
        raise ValueError(f'Cannot extract information from {sample} using {Data.regex}')

    [brem, block] = mtch.groups()

    return brem, block
#-------------------------------------
def _get_df(sample : str) -> pnd.DataFrame:
    path_wc = f'{Data.inp_dir}/{sample}/*/parameters.json'
    l_path  = glob.glob(path_wc)
    nfiles  = len(l_path)

    df = pnd.DataFrame(columns=['mu_val', 'mu_err', 'sg_val', 'sg_err', 'brem', 'block'])
    if nfiles == 0:
        raise ValueError(f'Cannot find any parameters file in: {path_wc}')


    log.info(f'Found {nfiles} parameters files')
    for path in l_path:
        df.loc[len(df)] = _row_from_path(path=path)

    return df
#-------------------------------------
def _process_dataframe(df : pnd.DataFrame) -> None:
    l_df_scale = []
    for (block, brem), df_group in df.groupby(['block', 'brem']):
        df_scale          = _scales_from_df(df=df_group)
        df_scale['block'] = block
        df_scale['brem' ] = brem

        l_df_scale.append(df_scale)

    df = pnd.concat(l_df_scale, ignore_index=True)

    print(df)
#-------------------------------------
def _scales_from_df(df : pnd.DataFrame) -> pnd.DataFrame:
    d_mu = _get_scale(df=df, name='mu', fun=lambda x : x[0] - x[1])
    d_sg = _get_scale(df=df, name='sg', fun=lambda x : x[0] / x[1])
    df   = pnd.DataFrame({**d_mu, **d_sg})

    return df
#-------------------------------------
def _get_scale(df : pnd.DataFrame, name : str, fun : Callable) -> dict[str:float]:
    df_dat  = df[df['sample'] == 'dat']
    df_sim  = df[df['sample'] == 'sim']

    dat_val = float(df_dat[f'{name}_val'])
    dat_err = float(df_dat[f'{name}_err'])

    sim_val = float(df_sim[f'{name}_val'])
    sim_err = 0# float(df_sim[f'{name}_err'])

    cov     = [
            [dat_err ** 2,            0],
            [0           , sim_err ** 2]]

    val, var = jacobi.propagate(fun, [dat_val, sim_val], cov)
    val      = float(val)
    err      = math.sqrt(var)

    if name == 'sg' and err > 100:
        log.warning('-----------------')
        log.info(f'Error: {err:.0f}')
        print(df)
        log.warning('-----------------')

    return {f's{name}_val' : [val], f's{name}_err' : [err]}
#-------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    _initialize()
    out_path = f'{Data.inp_dir}/parameters.json'
    if os.path.isfile(out_path):
        log.warning(f'Dataframe already found, reusing: {out_path}')
        df = pnd.read_json(out_path)
        _process_dataframe(df)

        return

    log.info('Dataframe already not found, making it')
    l_df = []
    for sample in ['dat', 'sim']:
        df           = _get_df(sample=sample)
        df['sample'] = sample
        l_df.append(df)

    df = pnd.concat(l_df, axis=0, ignore_index=True)
    df.fillna(0, inplace=True)

    df.to_json(out_path, indent=2)
    _process_dataframe(df)
#-------------------------------------
if __name__ == '__main__':
    main()
