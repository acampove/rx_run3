'''
Script used to plot mass distributions associated to samples in data and MC
used to study fully hadronic mis-ID backgrounds
'''
import argparse

import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_misid:plot_misid')
# ---------------------------------------
class Data:
    '''
    Data class
    '''
    file_path : str
    mass = 'B_M_brem_track_2'
    plt.style.use(mplhep.style.LHCb2)

    binning = {'bins' : 30, 'range' : (4500, 7000)}
# ---------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script meant to make plots for the samples used to study fully hadronic misID')
    parser.add_argument('-p','--path', type=str, help='Path to input file holding dataframe', required=True)
    args = parser.parse_args()

    Data.file_path = args.path
# ---------------------------------------
def _plot_kind(kind : str, df : pnd.DataFrame) -> None:
    ax = None
    ax = df.plot.hist(y=Data.mass, **Data.binning, alpha=0.3      , color='blue', label='Unweighed', ax=ax)
    ax = df.plot.hist(y=Data.mass, **Data.binning, histtype='step', color='red' , label='Weighted', ax=ax, weights=df['weights'])

    path = Data.file_path.replace('.parquet', f'_{kind}.png')

    plt.title(kind)
    plt.savefig(path)
    plt.close()
# ---------------------------------------
def _plot(df : pnd.DataFrame) -> None:
    for kind, df_kind in df.groupby('kind'):
        _plot_kind(kind=kind, df=df_kind)

    _plot_kind(kind='PF+FP+FF', df=df)
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
