'''
Script used to print contents of root files
'''

import argparse

from dmu.rfile.rfprinter import RFPrinter

# -----------------------------
class Data:
    '''
    Data class holding shared attributes
    '''
    path   : str
    screen : bool
# -----------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Script used to print information about ROOT files and dump it to text')
    parser.add_argument('-p', '--path'  , type=str, help='Path to ROOT file')
    parser.add_argument('-s', '--screen', help='If used, will dump output to screen', action='store_true')
    args = parser.parse_args()

    Data.path  = args.path
    Data.screen= args.screen
# -----------------------------
def main():
    '''
    Execution starts here
    '''
    _get_args()
    prt = RFPrinter(path = Data.path)
    prt.save(to_screen = Data.screen)
# -----------------------------
if __name__ == '__main__':
    main()
