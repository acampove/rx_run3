#!/usr/bin/python3
'''
Script used to link ntuples properly and merge them
'''

import argparse

from dataclasses import dataclass

# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    job : str
# ---------------------------------
def _get_args():
    '''
    Parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-j', '--job', type=str, help='Job name, e.g. flt_001', required=True) 
    args = parser.parse_args()

    Data.job = args.job
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
# ---------------------------------
if __name__ == '__main__':
    main()
