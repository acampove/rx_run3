'''
Script used to check which MC samples are found in grid
'''
import argparse

from dataclasses                          import dataclass
from ap_utilities.bookkeeping.bkk_checker import BkkChecker

# --------------------------------
@dataclass
class Data:
    '''
    Class storing shared attributes
    '''

    input_path  : str
    output_path : str
    nthread     : int
# --------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to filter samples based on what exists in the GRID')
    parser.add_argument('-i', '--input'  , type=str, help='Path to input YAML file')
    parser.add_argument('-n', '--nthread', type=int, help='Number of threads')
    args = parser.parse_args()

    Data.input_path  = args.input
    Data.output_path = args.input.replace('.yaml', '_found.yaml')
    Data.nthread     = args.nthread
# --------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()

    obj=BkkChecker(Data.input_path)
    obj.save(path=Data.output_path)
# --------------------------------
if __name__ == '__main__':
    main()
