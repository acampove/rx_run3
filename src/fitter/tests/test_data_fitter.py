'''
Module used to test DataFitter
'''
import pytest

from pathlib         import Path
from omegaconf       import OmegaConf
from dmu.stats.zfit  import zfit
from dmu.stats       import constraint_adder as cad 
from dmu.stats       import gof_calculator   as goc
from dmu.stats       import utilities        as sut
from dmu.generic     import utilities        as gut
from dmu.workflow    import Cache
from dmu             import LogStore
from fitter          import DataFitter
from fitter          import ToyMaker
from fitter          import ToyPlotter

log=LogStore.add_logger('fitter:test_data_fitter')

_sel_cfg = {
    'selection' : {'default' : {}, 'fit' : {}}
}

_constraints : dict[str,tuple[float,float]]= {
    'mu' : (5280, 10),
    'sg' : (  10,  1),
}

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:stats:gofcalculator', 30)
# ----------------------
def test_single_region(tmp_path : Path) -> None:
    '''
    Test fitting with single signal region
    '''
    pdf = sut.get_model(kind='s+b')
    dat = pdf.create_sampler(10_000)
    nll = zfit.loss.ExtendedUnbinnedNLL(data=dat, model=pdf)

    sel_cfg = OmegaConf.create(obj=_sel_cfg)
    d_nll   = {'signal_region' : (nll, sel_cfg)}

    cfg = gut.load_conf(package='fitter_data', fpath='tests/fits/single_region.yaml')
    with Cache.cache_root(path = tmp_path):
        ftr = DataFitter(
            name = 'single_region',
            d_nll= d_nll, 
            cfg  = cfg)
        ftr.run(kind='conf')
# ----------------------
def test_two_regions(tmp_path : Path) -> None:
    '''
    Test simultaneous fit with two regions
    '''
    obs     = None

    pdf_001 = sut.get_model(obs=obs, kind='s+b', suffix='001')
    dat_001 = pdf_001.create_sampler(10_000)
    nll_001 = zfit.loss.ExtendedUnbinnedNLL(data=dat_001, model=pdf_001)

    pdf_002 = sut.get_model(obs=obs, kind='s+b', suffix='002')
    dat_002 = pdf_002.create_sampler(10_000)
    nll_002 = zfit.loss.ExtendedUnbinnedNLL(data=dat_002, model=pdf_002)

    sel_cfg = OmegaConf.create(obj=_sel_cfg)
    d_nll   = {
        'region_001' : (nll_001, sel_cfg),
        'region_002' : (nll_002, sel_cfg),
    }

    with goc.GofCalculator.disabled(True),\
         Cache.cache_root(path = tmp_path):
        cfg = gut.load_conf(package='fitter_data', fpath='tests/fits/two_regions.yaml')
        ftr = DataFitter(
            name = 'two_regions',
            d_nll= d_nll, 
            cfg  = cfg)
        ftr.run(kind='conf')
# ----------------------
def test_two_regions_common_pars(tmp_path : Path) -> None:
    '''
    Test simultaneous fit with two regions
    and common parameters
    '''
    nsig    = zfit.param.Parameter('nsig', 0, 0, 1000_000)
    obs     = None

    pdf_001 = sut.get_model(obs=obs, kind='signal', suffix='001')
    pdf_001 = pdf_001.create_extended(yield_ = nsig)
    dat_001 = pdf_001.create_sampler(10_000)
    nll_001 = zfit.loss.ExtendedUnbinnedNLL(data=dat_001, model=pdf_001)

    pdf_002 = sut.get_model(obs=obs, kind='signal', suffix='002')
    pdf_002 = pdf_002.create_extended(yield_ = nsig)
    dat_002 = pdf_002.create_sampler(10_000)
    nll_002 = zfit.loss.ExtendedUnbinnedNLL(data=dat_002, model=pdf_002)

    sel_cfg = OmegaConf.create(obj=_sel_cfg)
    d_nll   = {
        'region_001' : (nll_001, sel_cfg),
        'region_002' : (nll_002, sel_cfg),
    }

    with goc.GofCalculator.disabled(True),\
         Cache.cache_root(path = tmp_path):
        cfg = gut.load_conf(package='fitter_data', fpath='tests/fits/two_regions.yaml')
        ftr = DataFitter(
            name = 'common_pars',
            d_nll= d_nll, 
            cfg  = cfg)
        ftr.run(kind='conf')
# ----------------------
def test_with_constraints(tmp_path : Path) -> None:
    '''
    Test fitting with constraints
    '''
    pdf = sut.get_model(kind='s+b')
    dat = pdf.create_sampler(10_000)
    nll = zfit.loss.ExtendedUnbinnedNLL(data=dat, model=pdf)

    sel_cfg = OmegaConf.create(obj=_sel_cfg)
    d_nll   = {'signal_region' : (nll, sel_cfg)}

    cns     = cad.ConstraintAdder.dict_to_cons(d_cns=_constraints, name='test', kind='GaussianConstraint')
    adr     = cad.ConstraintAdder(nll=nll, cns=cns)
    nll     = adr.get_nll() 

    cfg = gut.load_conf(package='fitter_data', fpath='tests/fits/single_region.yaml')
    cfg.constraints = cns

    with Cache.cache_root(path = tmp_path):
        ftr = DataFitter(
            name = 'with_const',
            d_nll= d_nll, 
            cfg  = cfg)

        ftr.run(kind='conf')
# ----------------------
slow_with_toys = pytest.param(500, marks=pytest.mark.slow)
@pytest.mark.parametrize('ntoys', [20, slow_with_toys])
def test_with_toys(ntoys : int, tmp_path : Path) -> None:
    '''
    Integration test

    - Fitting 
    - Making toys
    - Plotting
    '''
    pdf = sut.get_model(kind='s+b')
    dat = pdf.create_sampler(10_000)
    nll = zfit.loss.ExtendedUnbinnedNLL(data=dat, model=pdf)

    sel_cfg = OmegaConf.create(obj=_sel_cfg)
    d_nll   = {'signal_region' : (nll, sel_cfg)}

    fit_cfg = gut.load_conf(package='fitter_data', fpath='tests/fits/single_region.yaml')
    with Cache.cache_root(path = tmp_path):
        ftr = DataFitter(
            name = 'with_toys',
            d_nll= d_nll, 
            cfg  = fit_cfg)
        res = ftr.run(kind='zfit')

    with gut.environment(mapping = {'ANADIR' : str(tmp_path)}):
        toy_cfg = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_maker.yaml')
        toy_cfg.ntoys = ntoys
        mkr = ToyMaker(nll=nll, res=res, cfg=toy_cfg)
        df  = mkr.get_parameter_information()

        plt_cfg = gut.load_conf(package='fitter_data', fpath='tests/toys/toy_plotter_integration.yaml')
        ptr = ToyPlotter(df=df, cfg=plt_cfg)
        ptr.plot()
# ----------------------
