'''
Script used to plot histograms made by PIDCalib2
'''
import os
import re
import glob
import pickle
import argparse
from typing import TypeAlias

import numpy
import matplotlib.pyplot as plt
import mplhep
from dmu.logging.log_store import LogStore

Npa : TypeAlias = numpy.ndarray

log=LogStore.add_logger('rx_pid:plot_histograms')
# ---------------------------------
class Data:
    '''
    Data class
    '''
    dir_path   : str
    figsize    : tuple[int,int]
    fontsize   : int
    fancy      : bool = True
    skip_values: bool = True
    regex      : str  = r'effhists-2024_WithUT_block\d(?:_v1)?-(up|down)-([A-Z,a-z,0-9]+)-([A-Z,a-z,0-9,<,>,\.,&,\(,\)]+)-(\w+)\.(\w+)\.pkl'
# ---------------------------------
def _initialize() -> None:
    plt.style.use(mplhep.style.LHCb2)

    if Data.fancy:
        Data.figsize  = 12, 12
        Data.fontsize = 20
    else:
        Data.figsize  = 8, 8
        Data.fontsize = 15
# ------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to plot histograms in pkl files created by PIDCalib2')
    parser.add_argument('-d', '--dir_path', type=str, help='Path to directory with PKL files')
    args   = parser.parse_args()

    Data.dir_path = args.dir_path
# ------------------------------------
def _get_pkl_paths() -> list[str]:
    path_wc = f'{Data.dir_path}/*.pkl'
    l_path  = glob.glob(path_wc)
    npath   = len(l_path)

    if npath == 0:
        raise FileNotFoundError(f'No files found in {path_wc}')

    return l_path
# ------------------------------------
def _add_values(x_edges : Npa, y_edges : Npa, counts : Npa, errors : Npa) -> None:
    if Data.skip_values:
        return

    for i in range(len(x_edges)-1):
        for j in range(len(y_edges)-1):
            x_center = (x_edges[i] + x_edges[i+1]) / 2
            y_center = (y_edges[j] + y_edges[j+1]) / 2
            val      = counts[i][j]
            err      = errors[i][j]
            text     = f'{val:.2f}\n$\pm${err:.2f}'
            plt.text(x_center, y_center, text, ha='center', va='center', color='white', fontdict={'size': Data.fontsize})
# ------------------------------------
def _plot_hist(pkl_path : str) -> None:
    log.info(f'Plotting histograms in: {pkl_path}')
    with open(pkl_path, 'rb') as ifile:
        hist = pickle.load(ifile)

    x_edges    = hist.axes[0].edges
    y_edges    = hist.axes[1].edges
    bin_values = hist.view()
    counts     = bin_values['value']
    variances  = bin_values['variance']
    errors     = numpy.sqrt(variances)

    arr_x, arr_y = numpy.meshgrid(x_edges, y_edges)
    plt.pcolormesh(arr_x, arr_y, counts.T, shading='auto', vmin=0.0, vmax=1.)
    plt.colorbar(label='Efficiency')

    _add_values(x_edges, y_edges, counts, errors)
    _add_info(pkl_path)

    out_path = pkl_path.replace('.pkl', '.png')
    plt.savefig(out_path)
    plt.close()
# ------------------------------------
def _add_info(pkl_path : str) -> None:
    file_name = os.path.basename(pkl_path)
    mtch      = re.match(Data.regex, file_name)
    if not mtch:
        raise ValueError(f'Cannot extract information from {file_name}, using {Data.regex}')

    [pol, par, cut, xlabel, ylabel] = mtch.groups()

    title = f'{par}; {pol}; {cut}'

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
# ------------------------------------
def main():
    '''
    start here
    '''
    _parse_args()
    _initialize()

    l_path = _get_pkl_paths()
    for pkl_path in l_path:
        _plot_hist(pkl_path)
# ------------------------------------
if __name__ == "__main__":
    main()
