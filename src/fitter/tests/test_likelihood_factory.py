'''
Module meant to test DataFitter class
'''
import pytest

from pathlib               import Path
from dmu.stats.fitter      import LogStore
from dmu.stats.parameters  import ParameterLibrary as PL
from dmu.stats.zfit        import zfit
from dmu.generic           import utilities  as gut
from dmu.workflow          import Cache
from rx_common.types       import Trigger
from rx_data               import RDFGetter
from rx_selection          import selection  as sel
from fitter                import LikelihoodFactory

# -------------------------------------------
class Data:
    '''
    Used to share attributes
    '''
    d_block_cut = {
        'b12': 'block == 1 || block == 2',
        'b3' : 'block == 3',
        'b4' : 'block == 4',
        'b5' : 'block == 5',
        'b6' : 'block == 6',
        'b78': 'block == 7 || block == 8'}
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_data:rdf_getter'      , 30)
    LogStore.set_level('rx_misid:sample_weighter', 30)
    LogStore.set_level('fitter:LikelihoodFactory', 10)

    with RDFGetter.max_entries(value = 10_000):
        yield
# -------------------------------------------
def test_simple(tmp_path : Path):
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='tests/likelihood_factory/data.yaml')

    obs = zfit.Space('B_Mass_smr', limits=(5000, 6000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         Cache.cache_root(path = tmp_path),\
         RDFGetter.max_entries(value=100_000):

        ftr = LikelihoodFactory(
            name   = 'brem_000',
            obs    = obs,
            sample = 'DATA_24_MagDown_24c2',
            q2bin  = 'jpsi',
            cfg    = cfg)
        nll = ftr.run()


# -------------------------------------------
def test_config(tmp_path : Path):
    '''
    Tests the config method
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/rk/muon/data.yaml')

    obs = zfit.Space('B_Mass', limits=(4500, 7000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {'bdt' : '(1)'}),\
         Cache.cache_root(path = tmp_path),\
         RDFGetter.max_entries(value=100_000):

        ftr = LikelihoodFactory(
            name   = 'likelihood_factory',
            obs    = obs,
            sample = 'DATA_24_MagDown_24c2',
            trigger= Trigger.rk_mm_os,
            q2bin  = 'jpsi',
            cfg    = cfg)
        cfg = ftr.get_config()

    assert 'selection' in cfg
    assert 'default'   in cfg.selection
    assert 'fit'       in cfg.selection

    sel_def = cfg.selection.default
    sel_fit = cfg.selection.fit

    assert sel_def.keys() == sel_fit.keys()
    assert sel_def.bdt    != sel_fit.bdt
    assert sel_fit.bdt    == '(1)'
# -------------------------------------------
def test_reso_muon(tmp_path : Path):
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/rk/muon/data.yaml')

    obs = zfit.Space('B_Mass_smr', limits=(5000, 6000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {'bdt' : '(1)'}), \
         Cache.cache_root(path = tmp_path),\
         RDFGetter.max_entries(value=100_000):

        ftr = LikelihoodFactory(
            name   = 'brem_000',
            obs    = obs,
            sample = 'DATA_24_MagDown_24c2',
            trigger= Trigger.rk_mm_os,
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['central'])
def test_rare_muon(q2bin : str, tmp_path : Path):
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/rk/muon/data.yaml')

    obs = zfit.Space('B_Mass', limits=(5000, 6000))
    with Cache.cache_root(path = tmp_path),\
         PL.parameter_schema(cfg=cfg.model.yields):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= Trigger.rk_mm_os,
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('nbrem', [1, 2])
def test_reso_electron(nbrem : int, tmp_path : Path):
    '''
    Test fitting resonant electron channel
    '''
    cfg   = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/rk/electron/data.yaml')

    obs = zfit.Space('B_const_mass_M', limits=(4800, 6000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         RDFGetter.max_entries(value = 200_000),\
         Cache.cache_root(path = tmp_path),\
         sel.custom_selection(d_sel={
            'brm12' : f'nbrem == {nbrem}',
            'mass'  : '(1)'}):

        ftr = LikelihoodFactory(
            name   = f'brem_{nbrem:03}',
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= Trigger.rk_ee_os,
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_rare_electron(q2bin : str, tmp_path : Path):
    '''
    Test fitting rare electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/rk/electron/data.yaml')

    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         Cache.cache_root(path = tmp_path),\
         sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'bdt'   : 'mva_cmb > 0.60 && mva_prc > 0.40'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= Trigger.rk_ee_os,
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
def test_high_q2_track(tmp_path : Path):
    '''
    Test fitting rare electron in high q2
    with track based cut and adding brem 0 category
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/rk/electron/data.yaml')

    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))

    with PL.parameter_schema(cfg=cfg.model.yields),\
         Cache.cache_root(path = tmp_path),\
         sel.custom_selection(d_sel={
            'q2'    : 'q2_track > 14300000 && q2 < 22000000',
            'bdt'   : 'mva_cmb > 0.8 && mva_prc > 0.8'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= Trigger.rk_ee_os,
            q2bin  = 'high',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
#@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
@pytest.mark.parametrize('q2bin' , ['central'])
@pytest.mark.parametrize('region, tag_cut', [
    ('hdkk'  , 'PROBNN_K > 0.1'), 
    ('hdpipi', 'PROBNN_K < 0.1')])
def test_rare_misid_electron(
    q2bin   : str,
    region  : str, 
    tag_cut : str,
    tmp_path: Path):
    '''
    Test building likelihood in misid control regions
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='misid/rk/electron/data_misid.yaml')

    l1_in_cr = f'((L1_PROBNN_E < 0.2) || (L1_PID_E < 3.0)) && L1_{tag_cut}'
    l2_in_cr = f'((L2_PROBNN_E < 0.2) || (L2_PID_E < 3.0)) && L2_{tag_cut}'

    obs = zfit.Space(f'B_Mass_{region}', limits=(4500, 7000))

    with PL.parameter_schema(cfg=cfg.model.yields),\
         RDFGetter.max_entries(value = -1),\
         RDFGetter.multithreading(nthreads = 6),\
         Cache.cache_root(path = tmp_path),\
         sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'mass'  : '(1)',
            'pid_l' : f'({l1_in_cr}) && ({l2_in_cr})'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            name   = region,
            sample = 'DATA_24_*',
            trigger= Trigger.rk_ee_ext,
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
