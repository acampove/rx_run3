'''
Script used to make diagnostic plots from filtered ntuples
'''

import copy
import argparse

from dataclasses import dataclass

import mplhep
import matplotlib.pyplot as plt
# TODO: do we need read_selection here? Then we need to depend on rx_selection
import read_selection    as rs

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from dmu.plotting.plotter   import Plotter

import post_ap.utilities as ut

log=LogStore.add_logger('post_ap:plot_vars')
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
def _filter_rdf(rdf : RDataFrame, skip_bdt : bool) -> RDataFrame:
    if Data.cfg_nam in ['hlt_cmp_raw']:
        log.warning(f'Not applying any selection for {Data.cfg_nam}')
        return rdf
    bdt    = rs.get('bdt' , 'ETOS', 'none', '2024') if not skip_bdt else '(1)'
    mas    = rs.get('mass', 'ETOS', 'jpsi', '2024')
    rdf    = rdf.Define('BDT_prc', '1')
    rdf    = rdf.Filter(mas, 'mass')
    rdf    = rdf.Filter(bdt, 'BDT' )
    rep    = rdf.Report()
    rep.Print()

    return rdf
# -------------------------------------
def _get_rdf(path_wc : str, skip_bdt : bool) -> RDataFrame:
    '''
    Takes wildcard to ROOT files used as input
    Will return ROOT dataframe
    '''
    tree_name = Data.cfg_dat['input']['tree_name']

    log.debug(f'Loading: {path_wc}/{tree_name}')

    rdf    = RDataFrame(tree_name, path_wc)
    rdf    = _filter_rdf(rdf, skip_bdt)
    nev    = rdf.Count().GetValue()
    log.debug(f'Found {nev} entries in: {path_wc}')

    return rdf
# -------------------------------------
def _get_config(skip_bdt : bool) -> dict:
    '''
    Will pick skip_bdt flag and override plotting directory
    returns updated config
    '''

    cfg     = copy.deepcopy(Data.cfg_dat)
    plt_dir = cfg['saving']['plt_dir']

    if skip_bdt:
        cfg['saving']['plt_dir'] = f'{plt_dir}/no_bdt'
    else:
        cfg['saving']['plt_dir'] = f'{plt_dir}/with_bdt'

    return cfg
# -------------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    plt.style.use(mplhep.style.LHCb2)

    ut.local_config=True
    Data.cfg_dat = ut.load_config(Data.cfg_nam, kind='yaml')
    log_store.set_level('post_ap:plot_vars', Data.log_lvl)

    for skip_bdt in [True, False]:
        d_inp = Data.cfg_dat['input']['file_wc']
        d_rdf = { samp : _get_rdf(dset, skip_bdt) for samp, dset in d_inp.items()}

        cfg   = _get_config(skip_bdt)
        ptr   = Plotter(d_rdf, cfg)
        ptr.run()
# -------------------------------------
if __name__ == '__main__':
    main()
