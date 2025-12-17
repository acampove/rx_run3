'''
Script used to interact with DataFitter tool
and run fits
'''

import os
os.environ['CUDA_VISIBLE_DEVICES'] = '-1'

import argparse

from typing                     import Final
from contextlib                 import ExitStack
from omegaconf                  import DictConfig, OmegaConf
from dmu.stats.parameters       import ParameterLibrary as PL
from dmu.generic                import utilities as gut
from dmu.stats                  import utilities as sut
from dmu.stats.constraint_adder import ConstraintAdder
from dmu.workflow.cache         import Cache
from dmu.logging.log_store      import LogStore
from zfit.loss                  import ExtendedUnbinnedNLL

from fitter.fit_config         import FitConfig
from fitter.constraint_reader  import ConstraintReader
from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory
from fitter.misid_constraints  import MisIDConstraints 
from fitter.toy_maker          import ToyMaker
from rx_data.rdf_getter        import RDFGetter
from rx_selection              import selection as sel
from rx_common                 import info

log=LogStore.add_logger('fitter:fit_rx_rare')

DATA_SAMPLE : Final[str] = 'DATA_24_*'
# ----------------------
def _parse_args(args : DictConfig | argparse.Namespace | None = None) -> FitConfig:
    '''
    Returns
    --------------
    Instance of configuration class, built from arguments
    '''
    if args is None:
        parser = argparse.ArgumentParser(description='Script used to fit RX data')
        parser.add_argument('-b', '--block'  , type=int  , help='Block number, if not passed will do all data'    , choices =[1,2,3,4,5,6,7,8], default=-1)
        parser.add_argument('-g', '--group'  , type=str  , help='Name of group to which fit belongs, e.g. toys'   , required= True)
        parser.add_argument('-c', '--fit_cfg', type=str  , help='Name of configuration, e.g. rare/rk/electron'    , required= True)
        parser.add_argument('-t', '--toy_cfg', type=str  , help='Name of toy config, e.g. toys/maker.yaml'        , default =   '')
        parser.add_argument('-N', '--ntoys'  , type=int  , help='If specified, this will override ntoys in config', default =0)
        parser.add_argument('-n', '--nthread', type=int  , help='Number of threads'                               , default =1)
        parser.add_argument('-l', '--log_lvl', type=int  , help='Logging level', choices=[5, 10, 20, 30]          , default =20)
        parser.add_argument('-q', '--q2bin'  , type=str  , help='q2 bin'      , choices=['low', 'central', 'high'], required=True)
        parser.add_argument('-C', '--mva_cmb', type=float, help='Cut on combinatorial MVA working point'          , required=True)
        parser.add_argument('-P', '--mva_prc', type=float, help='Cut on part reco MVA working point'              , required=True)
        args = parser.parse_args()

    return _cfg_from_args(args = args)
# ----------------------
def _cfg_from_args(args : DictConfig | argparse.Namespace) -> FitConfig:
    '''
    Parameters
    -------------
    args: Object storing configuration

    Returns
    -------------
    Object storing full fit configuration
    '''
    fit_cfg = gut.load_conf(package='fitter_data', fpath=f'{args.fit_cfg}/data.yaml')
    toy_cfg = gut.load_conf(package='fitter_data', fpath=args.toy_cfg) if args.toy_cfg else None

    channel = info.channel_from_trigger(trigger = fit_cfg.trigger, lower_case=True)

    if channel   == 'ee':
        name     = 'brem_x12'
    elif channel == 'mm':
        name     = 'brem_0xx'
    else:
        raise NotImplementedError(f'Invalid channel: {channel}')

    cfg = FitConfig(
        name    = name,
        fit_cfg = fit_cfg, 
        toy_cfg = toy_cfg,
        group   = args.group,
        block   = args.block,
        q2bin   = args.q2bin,
        nthread = args.nthread,
        mva_cmb = args.mva_cmb / 100.,
        mva_prc = args.mva_prc / 100.,
        log_lvl = args.log_lvl,
        ntoys   = args.ntoys)

    return cfg
# ----------------------
def _use_constraints(
    kind : str,
    cfg  : FitConfig) -> bool:
    '''
    Parameters
    -------------
    kind: Label for constraints, e.g. misid
    cfg : Object holding configuration for fit 

    Returns
    -------------
    It will check in the config and will return true to run on these constraints
    E.g. components not in the model, do not need constraints
    '''
    if kind not in ['misid']:
        raise ValueError(f'Invalid kind: {kind}')

    l_misid   = ['kkk', 'kpipi']
    components= cfg.fit_cfg.model.components
    all_found = all(component in components for component in l_misid)

    if kind == 'misid' and all_found:
        return True

    return False
# ----------------------
def _get_constraints(
    nll : ExtendedUnbinnedNLL,
    cfg : FitConfig) -> DictConfig | None:
    '''
    Parameters
    -------------
    nll: Likelihood
    cfg: Object holding configuration for fit 

    Returns
    -------------
    Dictionary with:
        key  : Name of parameter
        Value: Tuple with mu and sigma for constraining parameter
    '''
    crd   = ConstraintReader(obj=nll, q2bin=cfg.q2bin)
    d_cns = crd.get_constraints()
    cons  = ConstraintAdder.dict_to_cons(d_cns=d_cns, name='scales', kind='GaussianConstraint')

    if _use_constraints(kind='misid', cfg=cfg):
        mrd       = MisIDConstraints(
            obs   = cfg.observable,
            cfg   = cfg.fit_cfg.model.constraints.misid,
            q2bin = cfg.q2bin)
        d_cns   = mrd.get_constraints()
        tmp     = ConstraintAdder.dict_to_cons(d_cns=d_cns, name='misid' , kind='PoissonConstraint')
        cons    = OmegaConf.merge(cons, tmp)
    else:
        log.info('Skipping misid constraints')

    if cons is None:
        log.warning('Not using any constraints')
        return cons

    if not isinstance(cons, DictConfig):
        raise ValueError('Configuration is not a DictConfig')

    log.info('Constraints:')
    cons_str = OmegaConf.to_yaml(cons)
    log.info('\n\n' + cons_str)

    return cons 
# ----------------------
def _fit(cfg : FitConfig) -> None:
    '''
    This is where DataFitter is used
    '''
    ftr = LikelihoodFactory(
        obs    = cfg.observable,
        q2bin  = cfg.q2bin,
        sample = DATA_SAMPLE,
        trigger= cfg.fit_cfg.trigger,
        cfg    = cfg.fit_cfg)
    nll = ftr.run()
    cfg_mod = ftr.get_config()

    cfg_cns = _get_constraints(nll=nll, cfg=cfg)
    cad     = ConstraintAdder(nll=nll, cns=cfg_cns)
    nll     = cad.get_nll()

    # Type analyser needs to be told this is the right type
    if not isinstance(nll, ExtendedUnbinnedNLL):
        raise ValueError('Likelihood is not extended and unbinned')

    cfg.fit_cfg['constraints'] = cfg_cns
    ftr = DataFitter(
        name = cfg.q2bin,
        d_nll= {cfg.name : (nll, cfg_mod)}, 
        cfg  = cfg.fit_cfg)
    res = ftr.run(kind='zfit')

    if cfg.toy_cfg is None:
        log.info('Not making toys')
        return

    cfg.toy_cfg['constraints'] = cfg_cns
    log.info(f'Making {cfg.toy_cfg.ntoys} toys')
    mkr = ToyMaker(nll=nll, res=res, cfg=cfg.toy_cfg)
    mkr.get_parameter_information()
# ----------------------
def main(args : DictConfig | None = None):
    '''
    Entry point
    '''
    cfg = _parse_args(args = args)

    with ExitStack() as stack:
        stack.enter_context(Cache.cache_root(path=cfg.output_directory))
        stack.enter_context(PL.parameter_schema(cfg=cfg.fit_cfg.model.yields))
        stack.enter_context(sel.custom_selection(d_sel=cfg.overriding_selection))
        stack.enter_context(RDFGetter.multithreading(nthreads=cfg.nthread))
        stack.enter_context(sut.blinded_variables(regex_list=['.*signal.*']))

        _fit(cfg=cfg)
# ----------------------
if __name__ == '__main__':
    main()
