'''
Script used to interact with DataFitter tool
and run fits
'''

import os
os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
import argparse
from typing import ClassVar

import yaml
from omegaconf                  import DictConfig, OmegaConf
from dmu.stats.zfit             import zfit
from dmu.stats.parameters       import ParameterLibrary as PL
from dmu.generic                import utilities as gut
from dmu.stats                  import utilities as sut
from dmu.stats.constraint_adder import ConstraintAdder
from dmu.workflow.cache         import Cache
from dmu.logging.log_store      import LogStore
from zfit.loss                  import ExtendedUnbinnedNLL
from zfit.interface             import ZfitSpace as zobs

from fitter.constraint_reader  import ConstraintReader
from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory
from fitter.misid_constraints  import MisIDConstraints 
from fitter.toy_maker          import ToyMaker
from rx_data.rdf_getter        import RDFGetter
from rx_selection              import selection as sel

log=LogStore.add_logger('fitter:fit_rx_data')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    fit_cfg : ClassVar[DictConfig]
    toy_cfg : DictConfig|None
    l_q2bin= ['low', 'cen_low', 'central', 'cen_high', 'jpsi', 'psi2', 'high']

    nthread : int   = 1
    q2bin   : str   = ''
    mva_cmb : float = 0.0
    mva_prc : float = 0.0
    log_lvl : int   = 20
    obs     : zobs
# ----------------------
def _set_logs() -> None:
    '''
    Will put classes in a given logging level
    '''
    LogStore.set_level('dmu:workflow:cache'                   ,           30)
    LogStore.set_level('dmu:stats:utilities'                  ,           30)
    LogStore.set_level('dmu:stats:model_factory'              ,           30)
    LogStore.set_level('dmu:stats:gofcalculator'              ,           30)
    LogStore.set_level('rx_data:rdf_getter'                   ,           30)
    LogStore.set_level('rx_efficiencies:efficiency_calculator',           30)
    LogStore.set_level('rx_selection:truth_matching'          ,           30)
    LogStore.set_level('rx_selection:selection'               ,           30)
    LogStore.set_level('fitter:prec'                          ,           30)
    LogStore.set_level('fitter:prec_scales'                   , Data.log_lvl)
    LogStore.set_level('fitter:constraint_reader'             , Data.log_lvl)
    LogStore.set_level('fitter:fit_rx_data'                   , Data.log_lvl)
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to fit RX data')
    parser.add_argument('-c', '--fit_cfg', type=str  , help='Name of configuration, e.g. rare/electron' , required=True)
    parser.add_argument('-t', '--toy_cfg', type=str  , help='Name of toy config, e.g. toys/maker.yaml'  , default =  '')
    parser.add_argument('-N', '--ntoys'  , type=int  , help='If specified, this will override ntoys in config', default    =0)
    parser.add_argument('-n', '--nthread', type=int  , help='Number of threads'                 , default=Data.nthread)
    parser.add_argument('-l', '--log_lvl', type=int  , help='Logging level', choices=[10, 20, 30], default=Data.log_lvl)
    parser.add_argument('-q', '--q2bin'  , type=str  , help='q2 bin',              choices=Data.l_q2bin , required=True)
    parser.add_argument('-C', '--mva_cmb', type=float, help='Cut on combinatorial MVA working point'    , required=True)
    parser.add_argument('-P', '--mva_prc', type=float, help='Cut on part reco MVA working point'        , required=True)
    args = parser.parse_args()

    Data.q2bin   = args.q2bin
    Data.nthread = args.nthread
    Data.mva_cmb = args.mva_cmb
    Data.mva_prc = args.mva_prc
    Data.log_lvl = args.log_lvl
    Data.fit_cfg = gut.load_conf(package='fitter_data', fpath=f'{args.fit_cfg}/data.yaml')
    Data.obs     = _get_observable()

    toy_cfg      = gut.load_conf(package='fitter_data', fpath=args.toy_cfg) if args.toy_cfg else None
    Data.toy_cfg = _override_toy_cfg(toy_cfg = toy_cfg, ntoys=args.ntoys) 
# ----------------------
def _override_toy_cfg(
    ntoys   : int,
    toy_cfg : DictConfig|None) -> DictConfig|None:
    '''
    Parameters
    -------------
    ntoys  : Number of toys specified by user
    toy_cfg: Config dictionary used for toy generation

    Returns
    -------------
    Input config after checks and overriding of fields
    '''
    if toy_cfg is None:
        log.debug('No toy configuration passed, skipping toys')
        return None

    if ntoys != 0:
        log.warning(f'Overriding number of toys with {ntoys}')
        toy_cfg.ntoys = ntoys

    root_dir= _get_output_directory()
    out_dir = f'{root_dir}/{Data.fit_cfg.output_directory}/{Data.q2bin}'

    log.info(f'Sending toys to: {out_dir}')
    toy_cfg.out_dir = out_dir

    return toy_cfg
# ----------------------
def _get_observable() -> zobs:
    '''
    Returns
    -------------
    Zfit observable
    '''
    cfg_obs      = Data.fit_cfg.model.observable
    [minx, maxx] = cfg_obs.range
    obs = zfit.Space(cfg_obs.name, minx, maxx)

    return obs
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
def _get_constraints(nll : ExtendedUnbinnedNLL) -> DictConfig:
    '''
    Parameters
    -------------
    nll: Likelihood

    Returns
    -------------
    Dictionary with:
        key  : Name of parameter
        Value: Tuple with mu and sigma for constraining parameter
    '''
    crd     = ConstraintReader(obj=nll, q2bin=Data.q2bin)
    d_cns_1 = crd.get_constraints()

    mrd     = MisIDConstraints(
        obs   = Data.obs,
        cfg   = Data.fit_cfg.model.constraints.misid,
        q2bin = Data.q2bin)
    d_cns_2 = mrd.get_constraints()

    d_cns = {**d_cns_1, **d_cns_2}
    cons  = ConstraintAdder.dict_to_cons(d_cns=d_cns, name='poisson', kind='PoissonConstraint')

    log.info('Constraints:')
    cons_str = OmegaConf.to_yaml(cons)
    log.info('\n\n' + cons_str)

    return cons 
# ----------------------
def _fit() -> None:
    '''
    This is where DataFitter is used
    '''
    ftr = LikelihoodFactory(
        obs    = Data.obs,
        q2bin  = Data.q2bin,
        sample = 'DATA_24_*',
        trigger= 'Hlt2RD_BuToKpEE_MVA',
        project= 'rx',
        cfg    = Data.fit_cfg)
    nll = ftr.run()
    cfg = ftr.get_config()

    cfg_cns = _get_constraints(nll=nll)
    cad     = ConstraintAdder(nll=nll, cns=cfg_cns)
    nll     = cad.get_nll(mode='real')

    # Type analyser needs to be told this is the right type
    if not isinstance(nll, ExtendedUnbinnedNLL):
        raise ValueError('Likelihood is not extended and unbinned')

    Data.fit_cfg['constraints'] = cfg_cns
    ftr = DataFitter(
        name = Data.q2bin,
        d_nll= {'' : (nll, cfg)}, 
        cfg  = Data.fit_cfg)
    res = ftr.run(kind='zfit')

    if Data.toy_cfg is None:
        log.info('Not making toys')
        return

    Data.toy_cfg['constraints'] = cfg_cns
    log.info(f'Making {Data.toy_cfg.ntoys} toys')
    mkr = ToyMaker(nll=nll, res=res, cfg=Data.toy_cfg)
    mkr.get_parameter_information()
# ----------------------
def _get_output_directory() -> str:
    '''
    Returns
    -----------------
    This function will return the directory WRT which
    the `output_directory` key in the fit config will be defined
    '''
    name    = _get_fit_name()
    ana_dir = os.environ['ANADIR']
    out_dir = f'{ana_dir}/fits/data/{name}'

    return out_dir
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    _set_logs()

    fit_name = _get_fit_name()
    out_dir  = _get_output_directory()
    Cache.set_cache_root(root=out_dir)
    with PL.parameter_schema(cfg=Data.fit_cfg.model.yields),\
         RDFGetter.multithreading(nthreads=Data.nthread),\
         RDFGetter.identifier(value=fit_name),\
         Cache.turn_off_cache(val=[]),\
         sut.blinded_variables(regex_list=['.*signal.*']),\
         sel.custom_selection(d_sel={
        'nobrm0': 'nbrem != 0',
        'bdt'   :f'(mva_cmb > {Data.mva_cmb}) && (mva_prc > {Data.mva_prc})'}):

        _fit()
# ----------------------
if __name__ == '__main__':
    main()
