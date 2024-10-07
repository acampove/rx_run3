'''
Script used to make diagnostic plots from filtered ntuples
'''

#!/usr/bin/env python3

import os
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
    l_dst   : list
    l_sam   : list

    log_lvl : int
    cfg_nam : str
    cfg_dat : dict

    year    : str
    vers    : str
# -------------------------------------
def _dst_sam_from_arg(l_dst):
    '''
    Will take a list of strings as:

    sample_identifier:name

    and will return a tuple with the identifier and the names
    '''

    l_idn = []
    l_nam = []
    for dst in l_dst:
        try:
            [idn, nam] = dst.split(':')
        except ValueError:
            log.error(f'Cannot find identifier and name in: {dst}')
            raise

        l_idn.append(idn)
        l_nam.append(nam)

    return l_idn, l_nam
# -------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to plot yields of cut based vs MVA based lines vs luminosity')
    parser.add_argument('-y', '--year', type =str, help='Year corresponding to dataset'                      , required=True)
    parser.add_argument('-v', '--vers', type =str, help='Version of dataset'                                 , required=True)
    parser.add_argument('-d', '--dst' , nargs='+', help='Type of dataset to plot, e.g. data_ana_cut_bp_ee'   , required=True)
    parser.add_argument('-c', '--cfg' , type =str, help='Name of config file specifying what to plot and how', required=True)
    parser.add_argument('-l', '--log' , type =int, help='Log level', default=20)
    args = parser.parse_args()

    Data.l_dst, Data.l_sam = _dst_sam_from_arg(args.dst)
    Data.year              = args.year
    Data.vers              = args.vers
    Data.cfg_nam           = args.cfg
    Data.log_lvl           = args.log
# -------------------------------------
def _define_vars(rdf):
    d_def = Data.cfg_dat['define_vars']
    log.info('Defining variables')
    for name, expr in d_def.items():
        log.debug(f'{name} <- {expr}')
        rdf = rdf.Define(name, expr)

    return rdf
# -------------------------------------
def _get_rdf(dset):
    '''
    Will take label of dataset, e.g. ctrl_BuToKpEE_ana_ee
    Will return ROOT dataframe
    '''
    tree_name = 'KMM' if 'MuMu' in dset else 'KEE'
    pfs_dir   = os.environ['PFSDIR']
    fpath     = f'{pfs_dir}/Data/RK/{dset}/{Data.vers}/{Data.year}.root'

    log.debug(f'Loading: {fpath}/{tree_name}')

    rdf    = RDataFrame(tree_name, fpath)
    rdf    = _define_vars(rdf)
    nev    = rdf.Count().GetValue()
    log.debug(f'Found {nev} entries in: {fpath}')

    return rdf
# -------------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    plt.style.use(mplhep.style.LHCb2)

    ut.local_config=True
    Data.cfg_dat = ut.load_config(Data.cfg_nam)
    log_store.set_level('data_checks:plot_vars', Data.log_lvl)

    d_rdf = { samp : _get_rdf(dset) for samp, dset in zip(Data.l_sam, Data.l_dst) }

    ptr   = Plotter(d_rdf=d_rdf, cfg=Data.cfg_dat)
    ptr.run()
# -------------------------------------
if __name__ == '__main__':
    main()
