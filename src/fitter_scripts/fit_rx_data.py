'''
Script used to interact with DataFitter tool
and run fits
'''

import os
import argparse
from typing import ClassVar

import yaml
from omegaconf                 import DictConfig
from dmu.stats.zfit            import zfit
from dmu.stats.parameters      import ParameterLibrary as PL
from dmu.generic               import utilities as gut
from dmu.stats                 import utilities as sut
from dmu.workflow.cache        import Cache
from dmu.logging.log_store     import LogStore
from zfit.interface            import ZfitLoss  as zloss
from zfit.interface            import ZfitSpace as zobs

from fitter.constraint_reader  import ConstraintReader
from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory
from fitter.misid_constraints  import MisIDConstraints 
from rx_data.rdf_getter        import RDFGetter
from rx_selection              import selection as sel

log=LogStore.add_logger('fitter:fit_rx_data')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    cfg    : ClassVar[DictConfig]
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
    parser.add_argument('-c', '--config' , type=str  , help='Name of configuration, e.g. rare/electron' , required=True)
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
    Data.cfg     = gut.load_conf(package='fitter_data', fpath=f'{args.config}/data.yaml')
    Data.obs     = _get_observable()
# ----------------------
def _get_observable() -> zobs:
    '''
    Returns
    -------------
    Zfit observable
    '''
    cfg_obs      = Data.cfg.model.observable
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
def _get_constraints(nll : zloss) -> dict[str,tuple[float,float]]:
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
        cfg   = Data.cfg.model.constraints.misid,
        q2bin = Data.q2bin)
    d_cns_2 = mrd.get_constraints()

    d_cns = {**d_cns_1, **d_cns_2}

    log.info('Constraints:')
    log.info(yaml.dump(d_cns))

    return d_cns
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
        cfg    = Data.cfg)
    nll = ftr.run()
    cfg = ftr.get_config()

    d_cns   = _get_constraints(nll=nll)

    ftr = DataFitter(
        name = Data.q2bin,
        d_nll= {'signal_region' : (nll, cfg)}, 
        cfg  = Data.cfg)
    ftr.constraints = d_cns 
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

    with PL.parameter_schema(cfg=Data.cfg.model.yields),\
         RDFGetter.multithreading(nthreads=Data.nthread),\
         Cache.turn_off_cache(val=[]),\
         sut.blinded_variables(regex_list=['.*signal.*']),\
         sel.custom_selection(d_sel={
        'nobrm0': 'nbrem != 0',
        'bdt'   :f'(mva_cmb > {Data.mva_cmb}) && (mva_prc > {Data.mva_prc})'}):

        _fit()
# ----------------------
if __name__ == '__main__':
    main()
