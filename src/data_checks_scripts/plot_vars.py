'''
Script used to make diagnostic plots from filtered ntuples
'''

#!/usr/bin/env python3

import argparse

from dataclasses import dataclass

import mplhep
import matplotlib.pyplot as plt

from ROOT                 import RDataFrame
from log_store            import log_store
from dmu.plotting.plotter import Plotter

import data_checks.utilities as ut


log=log_store.add_logger('data_checks:plot_vars')
# -------------------------------------
@dataclass
class Data:
    '''
    Class meant to store shared data
    '''
    log_lvl : int
    cfg_nam : str
    cfg_dat : dict
# -------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to plot yields of cut based vs MVA based lines vs luminosity')
    parser.add_argument('-c', '--cfg' , type =str, help='Name of config file specifying what to plot and how', required=True)
    parser.add_argument('-l', '--log' , type =int, help='Log level', default=20)
    args = parser.parse_args()

    Data.cfg_nam           = args.cfg
    Data.log_lvl           = args.log
# -------------------------------------
def _get_rdf(path_wc):
    '''
    Takes wildcard to ROOT files used as input
    Will return ROOT dataframe
    '''

    tree_name = Data.cfg_dat['input']['tree_name']

    log.debug(f'Loading: {path_wc}/{tree_name}')

    rdf    = RDataFrame(tree_name, path_wc)
    nev    = rdf.Count().GetValue()
    log.debug(f'Found {nev} entries in: {path_wc}')

    return rdf
# -------------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    plt.style.use(mplhep.style.LHCb2)

    ut.local_config=True
    Data.cfg_dat = ut.load_config(Data.cfg_nam, kind='yaml')
    log_store.set_level('data_checks:plot_vars', Data.log_lvl)

    d_inp = Data.cfg_dat['input']
    d_rdf = { samp : _get_rdf(dset) for samp, dset in d_inp.items()}

    ptr   = Plotter(d_rdf=d_rdf, cfg=Data.cfg_dat)
    ptr.run()
# -------------------------------------
if __name__ == '__main__':
    main()
