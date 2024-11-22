'''
Script used to validate ntuples produced by AP pipelines
'''
import argparse

from dataclasses import dataclass

# -------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''

    pipeline_id : int
    config_name : str
# -------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Makes a list of PFNs for a specific set of eventIDs in case we need to reprocess them')
    parser.add_argument('-p','--pipeline', type=int, help='Pipeline ID', required=True)
    parser.add_argument('-c','--config'  , type=str, help='Name of config file, without extension', required=True)
    args = parser.parse_args()

    Data.pipeline_id = args.pipeline
    Data.config_name = args.config
# -------------------------------
def _validate() -> None:
    ...
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
