'''
Script used to make diagnostic plots from filtered ntuples
'''
#!/usr/bin/env python3

import glob
import argparse

from functools   import cache
from dataclasses import dataclass

import tqdm
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT      import TFile , RDataFrame
from log_store import log_store

log = log_store.add_logger('data_checks:ntup_info')

# -------------------------------------
@dataclass
class Data:
    '''
    Class meant to store shared data
    '''
    l_trig = [
            'Hlt2RD_BuToKpEE',
            'Hlt2RD_BuToKpMuMu',
            'Hlt2RD_B0ToKpPimEE',
            'Hlt2RD_B0ToKpPimMuMu'
            'Hlt2RD_LbToLEE_LL',
            'Hlt2RD_LbToLMuMu_LL',
            ]

    dat_dir = None
# -------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to plot yields of cut based vs MVA based lines vs luminosity')
    parser.add_argument('-t', '--trig', nargs='+', help='HLT2 trigger', choices=Data.l_trig, default=Data.l_trig)
    parser.add_argument('-d', '--dir' , type =str, help='Path to ROOT files', required=True)
    parser.add_argument('-l', '--log' , type =int, help='Log level', default=20)
    args = parser.parse_args()

    Data.l_trig = args.trig
    Data.dat_dir= args.dir

    log.setLevel(args.log)
# -------------------------------------
@cache
def _get_paths():
    root_wc = f'{Data.dat_dir}/*.root'
    l_path  = glob.glob(root_wc)

    npath = len(l_path)
    if npath == 0:
        log.error(f'No ROOT files found in: {root_wc}')
        raise FileNotFoundError

    log.debug(f'Found {npath} files')

    return l_path
# -------------------------------------
def _get_rdf(trig):
    l_path = _get_paths()
    rdf = RDataFrame(trig, l_path)
    nev = rdf.Count().GetValue()
    log.debug(f'Found {nev} entries in: {trig}')

    return rdf
# -------------------------------------
def _plot_mass(rdf):
    arr_mass = rdf.AsNumpy(['B_const_mass_M'])['B_const_mass_M']
    plt.hist(arr_mass, bins=100, range=(5100, 6000))
    plt.axvline(x=5280, color='r', label='$B^+$', linestyle=':')
    plt.legend()
    plt.show()
# -------------------------------------
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
# -------------------------------------
def _get_info(path):
    ifile  = TFile(path)
    l_key  = ifile.GetListOfKeys()
    l_tre  = [ key.ReadObj() for key in l_key if 'Hlt2' in key.GetName() ]
    d_info = { tre.GetName() : tre.GetEntries() for tre in l_tre }
    ifile.Close()

    return d_info
# -------------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    _list_trees()

    for trig in Data.l_trig:
        rdf = _get_rdf(trig)
        _plot_mass(rdf)
# -------------------------------------
if __name__ == '__main__':
    main()
