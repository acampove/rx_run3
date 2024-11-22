'''
Script used to validate ntuples produced by AP pipelines
'''
import os

import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml

# -------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''

    pipeline_id : int
    config_name : str
    cfg         : dict
# -------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Makes a list of PFNs for a specific set of eventIDs in case we need to reprocess them')
    parser.add_argument('-p','--pipeline', type=int, help='Pipeline ID', required=True)
    parser.add_argument('-c','--config'  , type=str, help='Name of config file, without extension', required=True)
    args = parser.parse_args()

    Data.pipeline_id = args.pipeline
    Data.config_name = args.config
# -------------------------------
def _load_config() -> None:
    config_path = files('ap_utilities_data').joinpath(f'{Data.config_name}.yaml')
    config_path = str(config_path)

    if not os.path.isfile(config_path):
        raise FileNotFoundError(f'Could not find: {config_path}')

    with open(config_path, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)

    import pprint

    pprint.pprint(Data.cfg)
# -------------------------------
def _validate() -> None:
    _load_config()
# -------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _validate()
# -------------------------------
if __name__ == '__main__':
    main()
