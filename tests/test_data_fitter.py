'''
Module used to test DataFitter
'''
import pytest

from omegaconf             import OmegaConf
from dmu.stats.zfit        import zfit
from dmu.stats             import gof_calculator as goc
from dmu.stats             import utilities      as sut
from dmu.generic           import utilities      as gut
from dmu.logging.log_store import LogStore
from fitter.data_fitter    import DataFitter

log=LogStore.add_logger('fitter:test_data_fitter')

_sel_cfg = {
    'selection' : {'default' : {}, 'fit' : {}}
}

_constraints : dict[str,tuple[float,float]]= {
    'mu' : (5280, 10),
    'sg' : (  10,  1),
}
# ----------------------
def test_single_region() -> None:
    '''
    Test fitting with single signal region
    '''
    pdf = sut.get_model(kind='s+b')
    dat = pdf.create_sampler(10_000)
    nll = zfit.loss.ExtendedUnbinnedNLL(data=dat, model=pdf)

    sel_cfg = {'default' : {}, 'fit' : {}}
    sel_cfg = OmegaConf.create(obj=sel_cfg)
    d_nll   = {'signal_region' : (nll, sel_cfg)}

    cfg = gut.load_conf(package='fitter_data', fpath='tests/single_region.yaml')
    ftr = DataFitter(d_nll=d_nll, cfg=cfg)
    ftr.run()
# ----------------------
def test_two_regions() -> None:
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

    sel_cfg = {'default' : {}, 'fit' : {}}
    sel_cfg = OmegaConf.create(obj=sel_cfg)
    d_nll   = {
        'region_001' : (nll_001, sel_cfg),
        'region_002' : (nll_002, sel_cfg),
    }

    with goc.GofCalculator.disabled(True):
        cfg = gut.load_conf(package='fitter_data', fpath='tests/two_regions.yaml')
        ftr = DataFitter(d_nll=d_nll, cfg=cfg)
        ftr.run()
# ----------------------
def test_two_regions_common_pars() -> None:
    '''
    Test simultaneous fit with two regions
    and common parameters
    '''
    obs     = None

    pdf_001 = sut.get_model(obs=obs, kind='s+b', suffix='001')
    dat_001 = pdf_001.create_sampler(10_000)
    nll_001 = zfit.loss.ExtendedUnbinnedNLL(data=dat_001, model=pdf_001)

    pdf_002 = sut.get_model(obs=obs, kind='s+b', suffix='002')
    dat_002 = pdf_002.create_sampler(10_000)
    nll_002 = zfit.loss.ExtendedUnbinnedNLL(data=dat_002, model=pdf_002)

    sel_cfg = {'default' : {}, 'fit' : {}}
    sel_cfg = OmegaConf.create(obj=sel_cfg)
    d_nll   = {
        'region_001' : (nll_001, sel_cfg),
        'region_002' : (nll_002, sel_cfg),
    }

    with goc.GofCalculator.disabled(True):
        cfg = gut.load_conf(package='fitter_data', fpath='tests/two_regions.yaml')
        ftr = DataFitter(d_nll=d_nll, cfg=cfg)
        ftr.run()
# ----------------------
def test_with_constraints() -> None:
    '''
    Test fitting with constraints
    '''
    pdf = sut.get_model(kind='s+b')
    dat = pdf.create_sampler(10_000)
    nll = zfit.loss.ExtendedUnbinnedNLL(data=dat, model=pdf)

    sel_cfg = OmegaConf.create(obj=_sel_cfg)
    d_nll   = {'signal_region' : (nll, sel_cfg)}

    cfg = gut.load_conf(package='fitter_data', fpath='tests/single_region.yaml')
    ftr = DataFitter(d_nll=d_nll, cfg=cfg)

    with pytest.raises(ValueError):
        ftr.constraints = {} 

    ftr.constraints = _constraints

    with pytest.raises(ValueError):
        ftr.constraints = _constraints

    ftr.run()
# ----------------------
