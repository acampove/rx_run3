'''
Script used to make diagnostic plots from filtered ntuples
'''

#!/usr/bin/env python3

import os
import glob
import argparse

from functools   import cache
from dataclasses import dataclass

import ROOT
import mplhep
import matplotlib.pyplot as plt

from log_store            import log_store

import data_checks.utilities as ut

from data_checks.selector import selector

log=log_store.add_logger('data_checks:yield_vs_lumi')
#-------------------------------------
@dataclass
class Data:
    '''
    Class meant to store shared data
    '''
    l_trig = [
            'Hlt2RD_BuToKpEE',
            'Hlt2RD_BuToKpMuMu',
            'Hlt2RD_B0ToKpPimEE',
            'Hlt2RD_B0ToKpPimMuMu',
            'Hlt2RD_LbToLEE_LL',
            'Hlt2RD_LbToLMuMu_LL',
            ]

    log_lvl : int
    dat_dir : str
    plt_dir : str
    cfg_nam : str
    cfg_dat : dict
#-------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to plot yields of cut based vs MVA based lines vs luminosity')
    parser.add_argument('-t', '--trig', nargs='+', help='HLT2 trigger', choices=Data.l_trig, default=Data.l_trig)
    parser.add_argument('-d', '--dir' , type =str, help='Path to ROOT files', required=True)
    parser.add_argument('-l', '--log' , type =int, help='Log level', default=20)
    args = parser.parse_args()

    Data.l_trig = args.trig
    Data.dat_dir= args.dir
    Data.log_lvl= args.log
#-------------------------------------
def _set_logs():
    log_store.set_level('data_checks:yield_vs_lumi', Data.log_lvl)
    if Data.log_lvl == 10:
        log_store.set_level('data_checks:selector', 10)
#-------------------------------------
@cache
def _get_paths():
    root_wc = f'{Data.dat_dir}/*.root'
    l_path  = glob.glob(root_wc)

    npath = len(l_path)
    if npath == 0:
        log.error(f'No ROOT files found in: {root_wc}')
        raise

    log.debug(f'Found {npath} files')

    return l_path
#-------------------------------------
def _define_vars(rdf):
    d_def = Data.cfg_dat['define_vars']
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

    obj   = selector(rdf=rdf, cfg_nam=Data.cfg_nam, is_mc=True)
    d_rdf = obj.run(as_cutflow=True)

    return d_rdf
#-------------------------------------
def _plot_var(d_rdf, var):
    plt.figure(var)
    for name, rdf in d_rdf.items():
        minx, maxx, bins = Data.cfg_dat['plots'][var]['binning']
        yscale           = Data.cfg_dat['plots'][var]['yscale' ]
        [xname, yname]   = Data.cfg_dat['plots'][var]['labels' ]

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

    plot_path = f'{Data.plt_dir}/{var}.png'
    log.info(f'Saving to: {plot_path}')
    plt.tight_layout()
    plt.savefig(plot_path)
    plt.close()
#-------------------------------------
def _initialize(trig):
    Data.cfg_nam = {
            'Hlt2RD_BuToKpEE'      : 'bukee_opt',
            'Hlt2RD_B0ToKpPimMuMu' : 'bdkstmm_opt',
            }[trig]

    ut.local_config=True
    Data.cfg_dat = ut.load_config(Data.cfg_nam)

    plt_dir = Data.cfg_dat['saving']['plt_dir']
    os.makedirs(plt_dir, exist_ok=True)
    ut.local_config=True

    Data.plt_dir = plt_dir

    plt.style.use(mplhep.style.LHCb2)
#-------------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    _set_logs()

    for trig in Data.l_trig:
        _initialize(trig)
        d_rdf = _get_rdf(trig)
        for var in Data.cfg_dat['plots']:
            _plot_var(d_rdf, var)
#-------------------------------------
if __name__ == '__main__':
    main()
