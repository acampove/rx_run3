'''
Script used to interact with DataFitter tool
and run fits
'''

import os
import argparse
from typing import ClassVar

from omegaconf             import DictConfig
from dmu.generic           import utilities as gut
from dmu.workflow.cache    import Cache
from dmu.logging.log_store import LogStore

from fitter.data_fitter    import DataFitter
from rx_selection          import selection as sel

log=LogStore.add_logger('fitter:fit_rx_data')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    config : ClassVar[DictConfig]
    l_q2bin= ['low', 'cen_low', 'central', 'cen_high', 'jpsi', 'psi2', 'high']

    q2bin  : str   = ''
    mva_cmb: float = 0.0
    mva_prc: float = 0.0
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to fit RX data')
    parser.add_argument('-c', '--config' , type=str  , help='Name of configuration, e.g. rare_electron', required=True)
    parser.add_argument('-q', '--q2bin'  , type=str  , help='q2 bin',              choices=Data.l_q2bin, required=True)
    parser.add_argument('-C', '--mva_cmb', type=float, help='Cut on combinatorial MVA working point'   , required=True)
    parser.add_argument('-P', '--mva_prc', type=float, help='Cut on part reco MVA working point'       , required=True)
    args = parser.parse_args()

    Data.q2bin   = args.q2bin
    Data.mva_cmb = args.mva_cmb
    Data.mva_prc = args.mva_prc
    Data.config  = gut.load_conf(package='fitter_data', fpath=f'{args.config}/data.yaml')
# ----------------------
def _fit() -> None:
    '''
    This is where DataFitter is used
    '''
    with Cache.turn_off_cache(val=['DataFitter']),\
         sel.custom_selection(d_sel={
        'nobr0' : 'nbrem != 0',
        'bdt'   :f'mva_cmb > {Data.mva_cmb} && mva_prc > {Data.mva_prc}'}):
        ftr = DataFitter(
            name   = '060_040',
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = Data.q2bin,
            cfg    = Data.config)
        ftr.run()
# ----------------------
def _set_output_directory() -> None:
    '''
    This function tells the Cache class where to
    put the outputs. i.e. where the fit outputs will go
    '''
    ana_dir = os.environ['ANADIR']
    out_dir = f'{ana_dir}/fits/data'
    Cache.set_cache_root(root=out_dir)
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    _set_output_directory()
    _fit()
# ----------------------
if __name__ == '__main__':
    main()
