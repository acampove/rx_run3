'''
This module is meant to test the SimFitter class
'''
import pytest

from pathlib                import Path
from dmu.stats.zfit         import zfit
from dmu.generic            import utilities as gut
from dmu.stats              import utilities as sut
from dmu.workflow.cache     import Cache
from rx_common.types        import Trigger
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection as sel
from fitter.sim_fitter      import SimFitter
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('fitter:test_sim_fitter')
# ---------------------------------------------------
@pytest.fixture(autouse=True)
def initialize():
    '''
    Runs before any test
    '''
    log.info('Disabling caching')
    with RDFGetter.max_entries(value = 100_000):
        yield
# ---------------------------------------------------
def test_nomc(tmp_path : Path):
    '''
    Test for components that have no MC associated
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))

    cfg = gut.load_conf(package='fitter_data', fpath='rare/rk/electron/combinatorial.yaml')
    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_nomc',
            component= 'combinatorial',
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'low')
        _ = ftr.get_model()
# ---------------------------------------------------
def test_nocat(tmp_path : Path):
    '''
    Test for components without categories, e.g. muon
    '''
    obs   = zfit.Space('B_Mass_smr', limits=(5000, 5800))
    cfg   = gut.load_conf(package='fitter_data', fpath='rare/rk/muon/signal.yaml')

    with Cache.cache_root(path = tmp_path),\
         RDFGetter.max_entries(value=30_000):
        ftr = SimFitter(
            name     = 'test_nocat',
            component= 'signal_muon',
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_mm_os,
            q2bin    = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
def test_with_cat(tmp_path : Path):
    '''
    Test for components with brem categories
    '''
    obs   = zfit.Space('B_Mass', limits=(4500, 7000))
    cfg   = gut.load_conf(package='fitter_data', fpath='rare/rk/electron/signal_parametric.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_with_cat',
            component= 'signal_electron',
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'jpsi')
        _ = ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['bdkstee', 'bukstee', 'bsphiee'])
def test_kde(component : str, tmp_path : Path):
    '''
    Test fitting with KDE
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{component}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_kde',
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'central')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.skip(reason='These tests require smear friend trees for noPID samples')
@pytest.mark.parametrize('component', ['kkk', 'kpipi'])
@pytest.mark.parametrize('q2bin'    , ['low', 'central', 'high'])
def test_misid(component : str, q2bin : str, tmp_path : Path):
    '''
    Test fitting misID simulation 
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{component}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_misid',
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_nopid,
            q2bin    = q2bin)
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('limits', ['wide', 'narrow'])
def test_ccbar_reso(limits : str, tmp_path : Path):
    '''
    Tests retriveval of PDF associated to ccbar inclusive decays
    '''
    tp_limits = {'wide' : (4500, 6000), 'narrow' : (5000, 6000)}[limits]
    component = 'ccbar'
    obs       = zfit.Space('B_const_mass_M', limits=tp_limits)
    cfg       = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{component}.yaml')

    out_dir   = f'{cfg.output_directory}/{limits}'
    cfg.output_directory = out_dir

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_ccbar_reso',
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'jpsi')
        pdf = ftr.get_model()

    assert pdf is not None

    sut.print_pdf(pdf)
# ---------------------------------------------------
def test_ccbar_rare(tmp_path : Path):
    '''
    Tests retriveval of PDF associated to ccbar inclusive decays
    for rare modes, i.e. without jpsi mass constraint
    '''
    component = 'ccbar'
    mass      = 'B_Mass'
    q2bin     = 'high'
    obs       = zfit.Space(mass, limits=(4500, 6000))
    cfg       = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{component}.yaml')

    with Cache.cache_root(path = tmp_path),\
        sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'bdt'   : 'mva_cmb > 0.8 && mva_prc > 0.8'}):
        ftr = SimFitter(
            name     = 'test_ccbar_rare',
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = q2bin)
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['jpsi', 'cabibbo'])
@pytest.mark.parametrize('brem'     , [1, 2])
def test_reso_rk_ee(component : str, brem : int, tmp_path : Path):
    '''
    Test electron resonant with rk trigger
    '''
    obs   = zfit.Space('B_const_mass_M', limits=(5000, 6900))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{component}.yaml')

    with Cache.cache_root(path = tmp_path),\
        sel.custom_selection(d_sel={
            'mass'  : '(1)',
            'nbrem' :f'nbrem == {brem}'}):
        ftr = SimFitter(
            name     = 'test_mc_reso',
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['jpsi', 'psi2'])
@pytest.mark.parametrize('q2bin'    , ['jpsi', 'psi2'])
def test_reso_rkst_mm(component : str, q2bin : str, tmp_path : Path):
    '''
    Test resonant jpsi and psi2S in rkst muon channel
    '''

    obs_name = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    obs = zfit.Space(obs_name, limits=(5000, 6000))

    cfg = gut.load_conf(package='fitter_data', fpath=f'reso/rkst/muon/{component}.yaml')

    with Cache.cache_root(path = tmp_path),\
         sel.custom_selection(d_sel={'mass'  : '(1)'}):
        ftr = SimFitter(
            name     = 'reso_rkst_mm',
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rkst_mm_os,
            q2bin   = q2bin)
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('name', ['name_001', 'name_002'])
def test_name(name : str, tmp_path : Path):
    '''
    Will run test and specify the name argument
    '''
    component = 'ccbar'
    obs       = zfit.Space('B_const_mass_M', limits=(4500, 6000))
    cfg       = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{component}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = name,
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.skip(reason='These tests require smear friend trees for noPID samples')
@pytest.mark.parametrize('component', ['kpipi', 'kkk'])
def test_weights(component : str, tmp_path : Path):
    '''
    Test fitting weighted MC sample
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'misid/rk/electron/{component}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_weights',
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_nopid,
            q2bin   = 'central')
        ftr.get_model()
# ---------------------------------------------------
