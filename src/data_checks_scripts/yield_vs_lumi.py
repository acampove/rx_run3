#!/usr/bin/env python3

import os
import tqdm
import glob
import ROOT
import mplhep
import pandas            as pnd
import argparse
import matplotlib.pyplot as plt
import data_checks.utilities as ut

from data_checks.selector import selector 
from functools            import cache
from log_store            import log_store

log=log_store.add_logger('data_checks:yield_vs_lumi')
#-------------------------------------
class data:
    l_trig = [
            'Hlt2RD_BuToKpEE', 
            'Hlt2RD_BuToKpMuMu', 
            'Hlt2RD_B0ToKpPimEE', 
            'Hlt2RD_B0ToKpPimMuMu',
            'Hlt2RD_LbToLEE_LL',
            'Hlt2RD_LbToLMuMu_LL',
            ]

    dat_dir = None
    log_lvl = None
    plt_dir = None
    cfg_dat = None 
    cfg_nam = None 
#-------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to plot yields of cut based vs MVA based lines vs luminosity')
    parser.add_argument('-t', '--trig', nargs='+', help='HLT2 trigger', choices=data.l_trig, default=data.l_trig)
    parser.add_argument('-d', '--dir' , type =str, help='Path to ROOT files', required=True)
    parser.add_argument('-l', '--log' , type =int, help='Log level', default=20)
    args = parser.parse_args()

    data.l_trig = args.trig
    data.dat_dir= args.dir
    data.log_lvl= args.log
#-------------------------------------
def _set_logs():
    log_store.set_level('data_checks:yield_vs_lumi', data.log_lvl)
    if data.log_lvl == 10:
        log_store.set_level('data_checks:selector', 10)
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
def _define_vars(rdf):
    d_def = data.cfg_dat['define_vars']
    for name, expr in d_def.items():
        rdf = rdf.Define(name, expr)

    return rdf
#-------------------------------------
def _get_rdf(trig):
    l_path = _get_paths()
    rdf = ROOT.RDataFrame(trig, l_path)
    rdf = _define_vars(rdf)
    nev = rdf.Count().GetValue()
    log.debug(f'Found {nev} entries in: {trig}')

    obj   = selector(rdf=rdf, cfg_nam=data.cfg_nam, is_mc=True)
    d_rdf = obj.run(as_cutflow=True)

    return d_rdf
#-------------------------------------
def _plot_run_number(rdf):
    arr_rn = rdf.AsNumpy(['RUNNUMBER'])['RUNNUMBER']
    arr_rn = (arr_rn - 200000)/1000

    plt.hist(arr_rn, bins=100)
    plt.show()
#-------------------------------------
def _plot_var(d_rdf, var):
    plt.figure(var)
    for name, rdf in d_rdf.items():
        minx, maxx, bins = data.cfg_dat['plots'][var]['binning']
        yscale           = data.cfg_dat['plots'][var]['yscale' ]
        [xname, yname]   = data.cfg_dat['plots'][var]['labels' ]

        arr_mass = rdf.AsNumpy([var])[var]
        plt.hist(arr_mass, bins=bins, range=[minx, maxx], histtype='step', label=name)
        plt.yscale(yscale)
        plt.xlabel(xname)
        plt.ylabel(yname)

    if var in ['B_const_mass_M', 'B_M']:
        plt.axvline(x=5280, color='r', label='$B^+$', linestyle=':')
    elif var == 'Jpsi_M':
        plt.axvline(x=3096, color='r', label='$J/\psi$', linestyle=':')

    plt.legend()

    plot_path = f'{data.plt_dir}/{var}.png'
    log.info(f'Saving to: {plot_path}')
    plt.tight_layout()
    plt.savefig(plot_path)
    plt.close()
#-------------------------------------
def _plot(rdf):
    _plot_run_number(rdf)
    _plot_mass(rdf)
    _plot_yield(rdf)
#-------------------------------------
def _initialize(trig):
    data.cfg_nam = {
            'Hlt2RD_BuToKpEE'      : 'bukee_opt',
            'Hlt2RD_B0ToKpPimMuMu' : 'bdkstmm_opt',
            }[trig]

    ut.local_config=True
    data.cfg_dat = ut.load_config(data.cfg_nam)

    plt_dir = data.cfg_dat['saving']['plt_dir']
    os.makedirs(plt_dir, exist_ok=True)
    ut.local_config=True

    data.plt_dir = plt_dir

    plt.style.use(mplhep.style.LHCb2)
#-------------------------------------
def main():
    _get_args()
    _set_logs()

    for trig in data.l_trig:
        _initialize(trig)
        d_rdf = _get_rdf(trig)
        for var in data.cfg_dat['plots']:
            _plot_var(d_rdf, var)
#-------------------------------------
if __name__ == '__main__':
    main()

