#!/usr/bin/env python3

import tqdm
import glob
import ROOT
import pandas            as pnd
import argparse
import matplotlib.pyplot as plt

from functools import cache
from log_store import log_store

log=log_store.add_logger('data_checks:ntup_info')
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
def _list_trees():
    l_path = _get_paths()
    d_nevs = {}
    d_ntre = {}
    s_name = set()
    for path in tqdm.tqdm(l_path, ascii=' -'):
        d_tree = _get_info(path)
        for name, nevs in d_tree.items():
            if name not in  d_nevs:
                d_nevs[name]  = nevs
                d_ntre[name]  = 1 
            else:
                d_nevs[name] += nevs
                d_ntre[name] += 1 

            s_name.add(name)

    d_data = {'name' : [], 'nevs' : [], 'ntre' : []}
    for name in s_name:
        nevs = d_nevs[name]
        ntre = d_ntre[name]
        d_data['name'].append(name)
        d_data['nevs'].append(nevs)
        d_data['ntre'].append(ntre)

    df = pnd.DataFrame(d_data)
    print(df)
#-------------------------------------
def _get_info(path):
    ifile  = ROOT.TFile(path)
    l_key  = ifile.GetListOfKeys()
    l_tre  = [ key.ReadObj() for key in l_key if 'Hlt2' in key.GetName() ]
    d_info = { tre.GetName() : tre.GetEntries() for tre in l_tre }
    ifile.Close()

    return d_info
#-------------------------------------
def main():
    _get_args()
    _list_trees()

    return
    for trig in data.l_trig:
        trig_cut= trig
        trig_mva= f'{trig}_MVA'

        #rdf_cut = _get_rdf(trig_cut)
        #rdf_mva = _get_rdf(trig_mva)

        #_plot_run_number(rdf_cut)
        #_plot_yield()
        #_plot_mass(rdf_cut)
#-------------------------------------
if __name__ == '__main__':
    main()
