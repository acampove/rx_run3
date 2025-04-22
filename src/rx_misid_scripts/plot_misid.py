'''
Script used to plot mass distributions associated to samples in data and MC
used to study fully hadronic mis-ID backgrounds
'''
import argparse

import pandas as pnd
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_misid:plot_misid')
# ---------------------------------------
class Data:
    '''
    Data class
    '''
    file_path : str
# ---------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script meant to make plots for the samples used to study fully hadronic misID')
    parser.add_argument('-p','--path', type=str, help='Path to input file holding dataframe', required=True)
    args = parser.parse_args()

    Data.file_path = args.path
# ---------------------------------------
def _plot(df : pnd.DataFrame) -> None:
    print(df)
# ---------------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()

    df = pnd.read_parquet(Data.file_path)
    _plot(df)
# ---------------------------------------
if __name__ == '__main__':
    main()
