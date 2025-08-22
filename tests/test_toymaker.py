'''
This module tests the class ToyMaker
'''
import pytest

from pathlib               import Path

from zfit.loss             import ExtendedUnbinnedNLL
from fitter.toy_plotter    import ToyPlotter
from fitter.toy_maker      import ToyMaker
from dmu.stats             import utilities as sut
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore
from dmu.stats.fitter      import Fitter

log=LogStore.add_logger('fitter:test_toymaker')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:stats:gofcalculator', 30)
    LogStore.set_level('dmu:statistics:fitter'  , 20)
    LogStore.set_level('fitter:toy_maker'       , 10)
# ----------------------
def test_simple(ntoys : int) -> None:
    '''
    Simplest test of ToyMaker

    Parameters 
    -------------
    ntoys : Mean to pick number from:
            pytest --ntoys XXX
    '''
    log.info('')
    nll   = sut.get_nll(kind='s+b')
    if not isinstance(nll, ExtendedUnbinnedNLL):
        raise ValueError('Likelihood is not unbinned and or extended')

    res, _= Fitter.minimize(nll=nll, cfg={})

    cfg   = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_maker.yaml')
    cfg.constraints = gut.load_conf(package='fitter_data', fpath='tests/fits/constraint_adder.yaml') 

    if ntoys > 0:
        log.warning(f'Using user defined number of toys: {ntoys}')
        cfg.ntoys = ntoys
    else:
        ntoys = cfg.ntoys
        log.info(f'Not overriding number of toys from config: {ntoys}')

    mkr   = ToyMaker(nll=nll, res=res, cfg=cfg)
    df    = mkr.get_parameter_information()
    l_col = ['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Converged']

    assert df.columns.to_list() == l_col

    pars  = nll.get_params()
    assert len(df) == cfg.ntoys * len(pars) 
# ----------------------
@pytest.mark.parametrize('use_constraints', [True, False])
def test_integration(
    ntoys           : int, 
    test_dir        : Path,
    use_constraints : bool) -> None:
    '''
    Makes toys and then plots using ToyPlotter

    Parameters 
    -------------
    ntoys          : Mean to pick number from:
                     pytest --ntoys XXX
    test_dir       : Where output plots will go
    use_constraints: If true it will load and use constraints in config
    '''
    log.info('')
    nll   = sut.get_nll(kind='s+b')
    if not isinstance(nll, ExtendedUnbinnedNLL):
        raise ValueError('Likelihood is not unbinned and or extended')

    res, _= Fitter.minimize(nll=nll, cfg={})

    cfg   = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_maker.yaml')
    if use_constraints:
        cfg.constraints = gut.load_conf(package='fitter_data', fpath='tests/fits/constraint_adder.yaml') 

    if ntoys > 0:
        log.warning(f'Using user defined number of toys: {ntoys}')
        cfg.ntoys = ntoys
    else:
        ntoys = cfg.ntoys
        log.info('Not overriding number of toys from config: {ntoys}')

    mkr = ToyMaker(nll=nll, res=res, cfg=cfg)
    df  = mkr.get_parameter_information()

    name= {True : 'constrained', False : 'unconstrained'}[use_constraints]
    cfg = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_plotter_integration.yaml')
    cfg.saving.plt_dir = test_dir/f'toymaker/integration/{name}'
    ptr = ToyPlotter(df=df, cfg=cfg)
    ptr.plot()
# ----------------------
@pytest.mark.parametrize('use_constraints', [True, False])
def test_profile(ntoys : int, use_constraints : bool) -> None:
    '''
    Test used for profiling

    Parameters 
    -------------
    ntoys : Mean to pick number from:
            pytest --ntoys XXX
    '''
    log.info('')
    nll   = sut.get_nll(kind='s+b')
    if not isinstance(nll, ExtendedUnbinnedNLL):
        raise ValueError('Likelihood is not unbinned and or extended')

    res, _= Fitter.minimize(nll=nll, cfg={})
    cfg   = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_maker.yaml')
    if use_constraints:
        cfg.constraints = gut.load_conf(package='fitter_data', fpath='tests/fits/constraint_adder.yaml') 

    cfg.ntoys = ntoys

    mkr= ToyMaker(nll=nll, res=res, cfg=cfg)
    mkr.get_parameter_information()
