'''
Script used to dump metadata stored in ROOT file to YAML file
'''
import json
import argparse

import yaml
from ROOT import TFile

# ------------------------------
class Data:
    '''
    Data class used to store shared data
    '''
    yaml_path = 'metadata.yaml'
    fpath : str

# ------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to dump YAML file with metadata from ROOT file')
    parser.add_argument('-f', '--fpath' , type=str, help='Path to ROOT file', required=True)
    args = parser.parse_args()

    Data.fpath = args.fpath
# ------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()

    ifile = TFile.Open(Data.fpath)
    if not hasattr(ifile, 'metadata'):
        ifile.ls()
        raise ValueError(f'metadata missing in: {Data.fpath}')

    metadata = ifile.metadata
    meta_str = metadata.GetString().Data()
    data     = json.loads(meta_str)
    with open(Data.yaml_path, 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(data, ofile, width=float('inf'), sort_keys=False)
# ------------------------------
if __name__ == '__main__':
    main()
