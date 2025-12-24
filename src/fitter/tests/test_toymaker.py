'''
This module tests the class ToyMaker
'''
import pytest
import mplhep
import matplotlib.pyplot as plt

from pathlib               import Path
from zfit.loss             import ExtendedUnbinnedNLL
from fitter                import ToyPlotter
from fitter                import ToyMaker
from dmu.stats             import ConstraintAdder, build_constraint
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

    plt.style.use(mplhep.style.LHCb2)

    LogStore.set_level('dmu:stats:gofcalculator', 30)
    LogStore.set_level('dmu:statistics:fitter'  , 20)
    LogStore.set_level('fitter:toy_maker'       , 10)
    LogStore.set_level('fitter:constraint_adder', 10)
# ----------------------
def test_simple(tmp_path: Path) -> None:
    '''
    Simplest test of ToyMaker
    '''
    log.info('')
    nll   = sut.get_nll(kind='s+b')
    cfg   = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_maker.yaml')
    data  = gut.load_data(package='fitter_data', fpath='tests/fits/constraint_adder.yaml') 
    cns   = [ build_constraint(data=block) for block in data.values() ] 

    assert isinstance(nll, ExtendedUnbinnedNLL)
    adr = ConstraintAdder(nll = nll, constraints = cns)
    nll = adr.get_nll()

    res, _ = Fitter.minimize(nll=nll, cfg={})

    with gut.environment(mapping = {'ANADIR' : str(tmp_path)}):
        mkr   = ToyMaker(nll=nll, res=res, cfg=cfg, cns = cns)
        df    = mkr.get_parameter_information()

    l_col = ['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Valid']

    assert df.columns.to_list() == l_col

    pars  = nll.get_params()
    assert len(df) == cfg.ntoys * len(pars) 
# ----------------------
@pytest.mark.parametrize('ntoys', [None, 1000])
def test_integration(
    tmp_path : Path,
    ntoys    : int | None) -> None:
    '''
    Makes toys and then plots using ToyPlotter

    Parameters 
    -------------
    tmp_path : Where output plots will go
    ntoys    : Number of toys to override what is in config
    '''
    log.info('')
    nll   = sut.get_nll(kind='s+b')
    assert isinstance(nll, ExtendedUnbinnedNLL)

    cfg  = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_maker.yaml')
    if ntoys:
        cfg.ntoys = ntoys

    data = gut.load_data(package='fitter_data', fpath='tests/fits/constraint_adder.yaml') 
    cns  = [ build_constraint(data=block) for block in data.values() ] 
    adr  = ConstraintAdder(nll = nll, constraints = cns)
    nll  = adr.get_nll()

    res, _ = Fitter.minimize(nll=nll, cfg={})

    mkr = ToyMaker(nll=nll, res=res, cfg=cfg, cns = cns)
    df  = mkr.get_parameter_information()

    cfg = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_plotter_integration.yaml')
    cfg.saving.plt_dir = tmp_path / 'toymaker/integration/plots'
    ptr = ToyPlotter(df=df, cfg=cfg)
    ptr.plot()
# ----------------------
