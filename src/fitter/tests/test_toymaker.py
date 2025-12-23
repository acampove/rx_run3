'''
This module tests the class ToyMaker
'''
import pytest

from pathlib               import Path
from zfit.loss             import ExtendedUnbinnedNLL
from fitter                import ToyPlotter
from fitter                import ToyMaker
from dmu.stats             import utilities as sut
from dmu.generic           import utilities as gut
from dmu.stats             import Fitter
from dmu                   import LogStore

log=LogStore.add_logger('fitter:test_toymaker')
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def ntoys() -> int:
    return 10
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:stats:gofcalculator', 30)
    LogStore.set_level('dmu:statistics:fitter'  , 20)
    LogStore.set_level('fitter:toy_maker'       , 10)
    LogStore.set_level('fitter:constraint_adder', 10)
# ----------------------
def test_simple(
    ntoys   : int,
    tmp_path: Path) -> None:
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

    with gut.environment(mapping = {'ANADIR' : str(tmp_path)}):
        mkr   = ToyMaker(nll=nll, res=res, cfg=cfg)
        df    = mkr.get_parameter_information()

    l_col = ['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Valid']

    assert df.columns.to_list() == l_col

    pars  = nll.get_params()
    assert len(df) == cfg.ntoys * len(pars) 
# ----------------------
def test_integration(
    ntoys           : int, 
    tmp_path        : Path) -> None:
    '''
    Makes toys and then plots using ToyPlotter

    Parameters 
    -------------
    ntoys          : Mean to pick number from:
                     pytest --ntoys XXX
    test_dir       : Where output plots will go
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
        log.info('Not overriding number of toys from config: {ntoys}')

    mkr = ToyMaker(nll=nll, res=res, cfg=cfg)
    df  = mkr.get_parameter_information()

    cfg = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_plotter_integration.yaml')
    cfg.saving.plt_dir = tmp_path/'toymaker/integration/plots'
    ptr = ToyPlotter(df=df, cfg=cfg)
    ptr.plot()
# ----------------------
def test_profile(ntoys : int) -> None:
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
    cfg.constraints = gut.load_conf(package='fitter_data', fpath='tests/fits/constraint_adder.yaml') 

    cfg.ntoys = ntoys

    mkr= ToyMaker(nll=nll, res=res, cfg=cfg)
    mkr.get_parameter_information()
