'''
Script used to interact with DataFitter tool
and run fits to the resonant mode
'''

import os

from dmu.stats.fitter import GofCalculator
os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
import argparse
from contextlib import ExitStack

from omegaconf                  import DictConfig
from dmu.stats.parameters       import ParameterLibrary as PL
from dmu.generic                import utilities as gut
from dmu.stats                  import utilities as sut
from dmu.workflow.cache         import Cache
from dmu.logging.log_store      import LogStore
from zfit.loss                  import ExtendedUnbinnedNLL

from fitter.fit_config         import FitConfig
from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory
from rx_data.rdf_getter        import RDFGetter
from rx_selection              import selection as sel

log=LogStore.add_logger('fitter:fit_rx_reso')
# ----------------------
def _parse_args() -> FitConfig:
    '''
    Returns
    --------------
    Instance of configuration class, built from arguments
    '''
    parser = argparse.ArgumentParser(description='Script used to fit RX data')
    parser.add_argument('-b', '--block'  , type=int  , help='Block number, if not passed will do all data'    , choices =[12,3,4,5,6,78], default=-1)
    parser.add_argument('-c', '--fit_cfg', type=str  , help='Name of configuration, e.g. rare/electron'       , required=True)
    parser.add_argument('-t', '--toy_cfg', type=str  , help='Name of toy config, e.g. toys/maker.yaml'        , default =  '')
    parser.add_argument('-N', '--ntoys'  , type=int  , help='If specified, this will override ntoys in config', default =0)
    parser.add_argument('-n', '--nthread', type=int  , help='Number of threads'                               , default =1)
    parser.add_argument('-l', '--log_lvl', type=int  , help='Logging level', choices=[5, 10, 20, 30]          , default =20)
    parser.add_argument('-q', '--q2bin'  , type=str  , help='q2 bin'      , choices=['jpsi', 'psi2']          , required=True)
    parser.add_argument('-C', '--mva_cmb', type=float, help='Cut on combinatorial MVA working point'          , required=True)
    parser.add_argument('-P', '--mva_prc', type=float, help='Cut on part reco MVA working point'              , required=True)
    args = parser.parse_args()

    fit_cfg = gut.load_conf(package='fitter_data', fpath=f'{args.fit_cfg}/data.yaml')
    toy_cfg = gut.load_conf(package='fitter_data', fpath=args.toy_cfg) if args.toy_cfg else None

    cfg         = FitConfig(
        fit_cfg = fit_cfg, 
        toy_cfg = toy_cfg,
        block   = args.block,
        q2bin   = args.q2bin,
        nthread = args.nthread,
        mva_cmb = args.mva_cmb,
        mva_prc = args.mva_prc,
        log_lvl = args.log_lvl,
        ntoys   = args.ntoys)

    return cfg
# ----------------------
def _get_nll(cfg : FitConfig) -> tuple[ExtendedUnbinnedNLL, DictConfig]:
    '''
    Parameters
    -------------
    cfg : Fit configuration

    Returns
    -------------
    Tuple with:
        - Negative log likelihood
        - Config related to model 
    '''
    ftr = LikelihoodFactory(
        name   = cfg.name,
        obs    = cfg.observable,
        q2bin  = cfg.q2bin,
        sample = 'DATA_24_*',
        project= 'rx',
        trigger= cfg.fit_cfg.trigger,
        cfg    = cfg.fit_cfg)
    nll = ftr.run()
    cfg_mod = ftr.get_config()

    if not isinstance(nll, ExtendedUnbinnedNLL):
        raise TypeError('Likelihood object is not an ExtendedUnbinnedNLL')

    return nll, cfg_mod
# ----------------------
def _fit_electron(cfg : FitConfig) -> None:
    '''
    This is where DataFitter is used
    '''
    d_nll = {}
    with sel.update_selection(d_sel = {'brem_cat' : 'nbrem == 1'}):
        cfg.name = 'brem_001'
        cfg.replace(substring='brem_xxx', value=cfg.name)
        d_nll[cfg.name] = _get_nll(cfg=cfg)

    with sel.update_selection(d_sel = {'brem_cat' : 'nbrem == 2'}):
        cfg.name = 'brem_002'
        cfg.replace(substring='brem_001', value=cfg.name)
        d_nll[cfg.name] = _get_nll(cfg=cfg)

    cfg.fit_cfg.output_directory = 'reso/electron/data'
    with GofCalculator.disabled(value=True):
        ftr = DataFitter(
            name = cfg.q2bin,
            d_nll= d_nll, 
            cfg  = cfg.fit_cfg)
        ftr.run(kind='zfit')
# ----------------------
def _fit_muon(cfg : FitConfig) -> None:
    '''
    This is where DataFitter is used
    '''
    d_nll    = {}
    cfg.name = 'brem_000'
    cfg.replace(substring='brem_000', value=cfg.name)
    d_nll[cfg.name] = _get_nll(cfg=cfg)

    cfg.fit_cfg.output_directory = 'reso/muon/data'
    with GofCalculator.disabled(value=True):
        ftr = DataFitter(
            name = cfg.q2bin,
            d_nll= d_nll, 
            cfg  = cfg.fit_cfg)
        ftr.run(kind='zfit')
# ----------------------
def main():
    '''
    Entry point
    '''
    cfg = _parse_args()

    overriding_selection = {
        'mass'  : '(1)',
        'block' : cfg.block_cut,
        'nobrm0': cfg.brem_cut,
        'bdt'   : cfg.mva_cut}

    Cache.set_cache_root(root=cfg.output_directory)
    with ExitStack() as stack:
        stack.enter_context(PL.parameter_schema(cfg=cfg.fit_cfg.model.yields))
        stack.enter_context(RDFGetter.multithreading(nthreads=cfg.nthread))
        stack.enter_context(Cache.turn_off_cache(val=[]))
        stack.enter_context(sut.blinded_variables(regex_list=['.*signal.*']))
        stack.enter_context(sel.custom_selection(d_sel=overriding_selection))

        if cfg.is_electron: 
            _fit_electron(cfg=cfg)
        else:
            _fit_muon(cfg=cfg)
# ----------------------
if __name__ == '__main__':
    main()
