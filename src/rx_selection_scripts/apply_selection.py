#!/usr/bin/env python3

'''
Script used to apply selections to ROOT files
provided by DaVinci
'''

import os
import argparse

import yaml
from rx_selection.cache_data import CacheData

# ----------------------------------------
def _get_config(path : str) -> dict:
    '''
    Takes path to config
    Return settings from YAML as dictionary
    '''
    if not os.path.isfile(path):
        raise FileNotFoundError(f'Cannot find config: {path}')

    with open(path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# ----------------------------------------
def _set_threads() -> None:
    '''
    Need to use one thread to lower memory usage
    '''
    os.environ['MKL_NUM_THREADS']     ='1'
    os.environ['OMP_NUM_THREADS']     ='1'
    os.environ['NUMEXPR_NUM_THREADS'] ='1'
    os.environ['OPENBLAS_NUM_THREADS']='1'
# ----------------------------------------
def _get_args() -> argparse.Namespace:
    '''
    Argument parsing happens here
    '''
    parser = argparse.ArgumentParser(description='Script used to apply selection and produce samples')
    parser.add_argument('-c', '--conf' , type= str, help='Path to YAML config file', required=True)
    parser.add_argument('-i', '--ipart', type= int, help='Part to process'         , required=True)
    parser.add_argument('-n', '--npart', type= int, help='Total number of parts'   , required=True)

    args = parser.parse_args()

    return args
# ----------------------------------------
def main():
    '''
    Script starts here
    '''
    _set_threads()
    args = _get_args()
    cfg  = _get_config(args.conf)

    obj  = CacheData(cfg = cfg, ipart = args.ipart, npart = args.npart)
    obj.save()
# ----------------------------------------
if __name__ == '__main__':
    main()
