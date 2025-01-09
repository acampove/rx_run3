'''
Script used to make validation plots 
'''
import argparse
from dataclasses import dataclass

# --------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    version : str
# --------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to validate ntuples')
    parser.add_argument('-v', '--version' , type=str, help='Version of validation configuration', required=True)
    args = parser.parse_args()

    Data.version = args.version
# --------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()

# --------------------------
if __name__ == '__main__':
    main()
