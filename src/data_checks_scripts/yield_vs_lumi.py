#!/usr/bin/env python3

import tqdm
import glob
import ROOT
import pandas            as pnd
import argparse
import matplotlib.pyplot as plt

from functools import cache
from log_store import log_store

log=log_store.add_logger('data_checks:yield_vs_lumi')
#-------------------------------------
class data:
    l_trig = [
            'Hlt2RD_BuToKpEE', 
            'Hlt2RD_BuToKpMuMu', 
            'Hlt2RD_B0ToKpPimEE', 
            'Hlt2RD_B0ToKpPimMuMu'
            'Hlt2RD_LbToLEE_LL',
            'Hlt2RD_LbToLMuMu_LL',
            ]

    dat_dir = None
#-------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to plot yields of cut based vs MVA based lines vs luminosity')
    parser.add_argument('-t', '--trig', nargs='+', help='HLT2 trigger', choices=data.l_trig, default=data.l_trig)
    parser.add_argument('-d', '--dir' , type =str, help='Path to ROOT files', required=True)
    parser.add_argument('-l', '--log' , type =int, help='Log level', default=20)
    args = parser.parse_args()

    data.l_trig = args.trig
    data.dat_dir= args.dir

    log.setLevel(args.log)
#-------------------------------------
@cache
def _get_paths():
    root_wc = f'{data.dat_dir}/*.root'
    l_path  = glob.glob(root_wc)

    npath = len(l_path)
    if npath == 0:
        log.error(f'No ROOT files found in: {root_wc}')
        raise

    log.debug(f'Found {npath} files')

    return l_path
#-------------------------------------
def _get_rdf(trig):
    l_path = _get_paths()
    rdf = ROOT.RDataFrame(trig, l_path)
    nev = rdf.Count().GetValue()
    log.debug(f'Found {nev} entries in: {trig}')

    return rdf
#-------------------------------------
def _plot_yield():
    pass
#-------------------------------------
def _plot_run_number(rdf):
    arr_rn = rdf.AsNumpy(['RUNNUMBER'])['RUNNUMBER']
    arr_rn = (arr_rn - 200000)/1000

    plt.hist(arr_rn, bins=100)
    plt.show()
#-------------------------------------
def _plot_mass(rdf):
    arr_mass = rdf.AsNumpy(['B_const_mass_M'])['B_const_mass_M']
    plt.hist(arr_mass, bins=100, range=[5100, 6000])
    plt.axvline(x=5280, color='r', label='$B^+$', linestyle=':')
    plt.legend()
    plt.show()
#-------------------------------------
def _plot(rdf):
    _plot_run_number(rdf)
    _plot_mass(rdf)
    _plot_yield(rdf)
#-------------------------------------
def main():
    _get_args()

    for trig in data.l_trig:
        rdf = _get_rdf(trig)
        _plot()
#-------------------------------------
if __name__ == '__main__':
    main()
