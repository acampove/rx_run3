'''
Module containing tests for MisIDConstraints class
'''
import pytest

from pathlib                  import Path
from rx_selection             import selection as sel
from rx_data                  import RDFGetter
from dmu.workflow             import Cache
from dmu.stats                import print_constraints
from dmu.stats.zfit           import zfit
from dmu                      import LogStore
from dmu.generic              import utilities     as gut
from fitter.misid_constraints import MisIDConstraints 

log=LogStore.add_logger('fitter:test_misid_constraints')
# --------------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:workflow:cache'      , 30)
    LogStore.set_level('fitter:sim_fitter'       , 30)
    LogStore.set_level('fitter:base_fitter'      , 30)
    LogStore.set_level('fitter:data_preprocessor', 10)
    LogStore.set_level('rx_misid:sample_weighter', 20)
# ----------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str, tmp_path : Path) -> None:
    '''
    Basic test for building misID component
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 6000))
    cfg = gut.load_conf(package='fitter_data', fpath='misid/rk/electron/data_misid.yaml')
    cfg.output_directory = 'misid_constraints' 

    with sel.custom_selection(d_sel={'nobrm0' : 'nbrem != 0'}),\
         RDFGetter.max_entries(value = -1),\
         RDFGetter.multithreading(nthreads=8),\
         Cache.cache_root(path = tmp_path):
            obj = MisIDConstraints(
                obs      = obs,
                cfg      = cfg,
                q2bin    = q2bin)
            constraints = obj.get_constraints()

    print_constraints(constraints)
# ----------------------
