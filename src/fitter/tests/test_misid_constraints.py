'''
Module containing tests for MisIDConstraints class
'''
import os
import pytest
import yaml

from pathlib                  import Path
from rx_selection             import selection as sel
from rx_data                  import RDFGetter
from dmu.workflow             import Cache
from dmu.stats.zfit           import zfit
from dmu.logging.log_store    import LogStore
from dmu.generic              import utilities     as gut
from fitter.misid_constraints import MisIDConstraints 

log=LogStore.add_logger('fitter:test_misid_constraints')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    rel_dir = 'misid_constraints'
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

    os.makedirs(Data.rel_dir, exist_ok=True)
# ----------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str) -> None:
    '''
    Basic test for building misID component
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 6000))
    cfg = gut.load_conf(package='fitter_data', fpath='misid/rk/electron/data_misid.yaml')
    cfg.output_directory = Data.rel_dir

    with sel.custom_selection(d_sel={'nobrm0' : 'nbrem != 0'}):
        with RDFGetter.multithreading(nthreads=8):
            obj = MisIDConstraints(
                obs      = obs,
                cfg      = cfg,
                q2bin    = q2bin)
            d_cns = obj.get_constraints()

    data = yaml.dump(d_cns)
    log.info(data)
# ----------------------
