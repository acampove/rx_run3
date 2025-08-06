'''
Script used to interact with DataFitter tool
and run fits
'''

import os
import argparse
from typing import ClassVar

from omegaconf                 import DictConfig
from dmu.stats.zfit            import zfit
from dmu.stats.parameters      import ParameterLibrary as PL
from dmu.generic               import utilities as gut
from dmu.workflow.cache        import Cache
from dmu.logging.log_store     import LogStore

from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory
from rx_selection              import selection as sel

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
    log_lvl: int   = 20
# ----------------------
def _set_logs() -> None:
    '''
    Will put classes in a given logging level
    '''
    LogStore.set_level('fitter:constraint_reader', Data.log_lvl)
    LogStore.set_level('fitter:fit_rx_data'      , Data.log_lvl)
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to fit RX data')
    parser.add_argument('-c', '--config' , type=str  , help='Name of configuration, e.g. rare/electron', required=True)
    parser.add_argument('-l', '--log_lvl', type=int  , help='Logging level', choices=[10, 20, 30], default=Data.log_lvl)
    parser.add_argument('-q', '--q2bin'  , type=str  , help='q2 bin',              choices=Data.l_q2bin, required=True)
    parser.add_argument('-C', '--mva_cmb', type=float, help='Cut on combinatorial MVA working point'   , required=True)
    parser.add_argument('-P', '--mva_prc', type=float, help='Cut on part reco MVA working point'       , required=True)
    args = parser.parse_args()

    Data.q2bin   = args.q2bin
    Data.mva_cmb = args.mva_cmb
    Data.mva_prc = args.mva_prc
    Data.log_lvl = args.log_lvl
    Data.config  = gut.load_conf(package='fitter_data', fpath=f'{args.config}/data.yaml')
# ----------------------
def _get_fit_name() -> str:
    '''
    Builds fit identifier from MVA working points
    '''
    cmb  = int(100 * Data.mva_cmb)
    prc  = int(100 * Data.mva_prc)
    name = f'{cmb:03d}_{prc:03d}'

    return name
# ----------------------
def _fit() -> None:
    '''
    This is where DataFitter is used
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))

    with PL.parameter_schema(cfg=Data.config.model.yields),\
         Cache.turn_off_cache(val=[]),\
         sel.custom_selection(d_sel={
        'nobr0' : 'nbrem != 0',
        'bdt'   :f'mva_cmb > {Data.mva_cmb} && mva_prc > {Data.mva_prc}'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = Data.q2bin,
            cfg    = Data.config)
        nll = ftr.run()
        cfg = ftr.get_config()

    ftr = DataFitter(d_nll={'signal_region' : (nll, cfg)}, cfg=Data.config)
    ftr.run()
# ----------------------
def _set_output_directory() -> None:
    '''
    This function tells the Cache class where to
    put the outputs. i.e. where the fit outputs will go
    '''
    name    = _get_fit_name()
    ana_dir = os.environ['ANADIR']
    out_dir = f'{ana_dir}/fits/data/{name}'
    Cache.set_cache_root(root=out_dir)
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    _set_logs()
    _set_output_directory()
    _fit()
# ----------------------
if __name__ == '__main__':
    main()
