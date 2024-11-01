'''
Script meant to do truth matching checks
'''
import os
import argparse

import yaml

# ----------------------------------
def _get_args() -> argparse.Namespace:
    '''
    Parse args
    '''
    parser = argparse.ArgumentParser(description='Script used to carry out checks on truth matching mechanisms for MC')
    parser.add_argument('-c', '--conf' , type=str, help='Path to config file', required=True)
    args = parser.parse_args()

    return args
# ----------------------------------
def _get_config(args : argparse.Namespace) -> dict:
    path = args.conf
    if not os.path.isfile(path):
        raise FileNotFoundError(f'Cannot find {path}')

    with open(path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# ----------------------------------
def _check(cfg : dict) -> None:
    ...
# ----------------------------------
def main():
    '''
    Script starts here
    '''
    args = _get_args()
    cfg  = _get_config(args)
    _check(cfg)
# ----------------------------------
if __name__ == '__main__':
    main()
