'''
This module is meant to test the SimFitter class
'''
import pytest

from contextlib             import ExitStack
from pathlib                import Path
from dmu                    import LogStore
from dmu.stats.zfit         import zfit
from dmu.generic            import utilities as gut
from dmu.stats              import utilities as sut
from dmu.workflow           import Cache
from rx_common              import Sample, Trigger
from rx_data                import RDFGetter
from rx_selection           import selection as sel
from fitter                 import SimFitter

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
            name    = 'test_nomc',
            sample  = Sample.undefined,
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
    sample= Sample.bpkpmm
    obs   = zfit.Space('B_Mass_smr', limits=(5000, 5800))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'rare/rk/muon/{sample}.yaml')

    with Cache.cache_root(path = tmp_path),\
         RDFGetter.max_entries(value=30_000):
        ftr = SimFitter(
            name     = 'test_nocat',
            sample   = Sample.bpkpmm,
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
    sample= Sample.bpkpjpsiee
    obs   = zfit.Space('B_Mass', limits=(4500, 6000))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{sample}.yaml')

    cuts  = {
        'q2' : '(1)',
        'cmb': 'mva_cmb > 0.85',
        'prc': 'mva_prc > 0.50',
        'mass': 'B_Mass > 4500 && B_Mass < 6000',
        'block': 'block == 1',
        'nobrm0': 'nbrem != 0',
        'brem_cat': 'nbrem == 2',
    }

    with ExitStack() as stack:
        stack.enter_context(RDFGetter.max_entries(value = -1))
        stack.enter_context(LogStore.level('dmu:stats:fitter', 10))
        stack.enter_context(Cache.cache_root(path = tmp_path))
        stack.enter_context(sel.custom_selection(d_sel = cuts))

        ftr = SimFitter(
            name    = 'test_with_cat',
            sample  = Sample.bpkpjpsiee,
            trigger = Trigger.rk_ee_os,
            obs     = obs,
            cfg     = cfg,
            q2bin   = 'jpsi')
        _ = ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('sample', [Sample.bdkstkpiee, Sample.bpkstkpiee, Sample.bsphiee])
def test_kde(sample : Sample, tmp_path : Path):
    '''
    Test fitting with KDE
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{sample}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_kde',
            sample   = sample,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'central')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.skip(reason='These tests require smear friend trees for noPID samples')
@pytest.mark.parametrize('sample', [Sample.bpkkk, Sample.bpkpipi])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
def test_misid(sample : Sample, q2bin : str, tmp_path : Path):
    '''
    Test fitting misID simulation 
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{sample}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_misid',
            sample   = sample,
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
    sample    = Sample.ccbar
    obs       = zfit.Space('B_const_mass_M', limits=tp_limits)
    cfg       = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{sample}.yaml')

    out_dir   = f'{cfg.output_directory}/{limits}'
    cfg.output_directory = out_dir

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = 'test_ccbar_reso',
            sample   = sample,
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
    sample    = Sample.ccbar
    mass      = 'B_Mass'
    q2bin     = 'high'
    obs       = zfit.Space(mass, limits=(4500, 6000))
    cfg       = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{sample}.yaml')

    with Cache.cache_root(path = tmp_path),\
        sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'bdt'   : 'mva_cmb > 0.8 && mva_prc > 0.8'}):
        ftr = SimFitter(
            name    = 'test_ccbar_rare',
            sample  = sample,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = q2bin)
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('sample', [Sample.bpkpjpsiee, Sample.bppipjpsiee])
@pytest.mark.parametrize('brem'  , [1, 2])
def test_reso_rk_ee(sample : Sample, brem : int, tmp_path : Path):
    '''
    Test electron resonant with rk trigger
    '''
    obs   = zfit.Space('B_const_mass_M', limits=(5000, 6900))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{sample}.yaml')

    with Cache.cache_root(path = tmp_path),\
        sel.custom_selection(d_sel={
            'mass'  : '(1)',
            'nbrem' :f'nbrem == {brem}'}):
        ftr = SimFitter(
            name    = 'test_mc_reso',
            sample  = sample,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('sample', [Sample.bdkstkpijpsimm, Sample.bdkstkpipsi2mm])
@pytest.mark.parametrize('q2bin' , ['jpsi', 'psi2'])
def test_reso_rkst_mm(
    sample   : Sample, 
    q2bin    : str, 
    tmp_path : Path):
    '''
    Test resonant jpsi and psi2S in rkst muon channel
    '''

    obs_name = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    obs = zfit.Space(obs_name, limits=(5000, 6000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'reso/rkst/muon/{sample}.yaml')

    with Cache.cache_root(path = tmp_path),\
         sel.custom_selection(d_sel={'mass'  : '(1)'}):
        ftr = SimFitter(
            name    = 'reso_rkst_mm',
            sample  = sample,
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
    sample    = Sample.ccbar
    obs       = zfit.Space('B_const_mass_M', limits=(4500, 6000))
    cfg       = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{sample}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name     = name,
            sample   = sample,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.skip(reason='These tests require smear friend trees for noPID samples')
@pytest.mark.parametrize('sample', [Sample.bpkkk, Sample.bpkpipi]) 
def test_weights(sample : Sample, tmp_path : Path):
    '''
    Test fitting weighted MC sample
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(package='fitter_data', fpath=f'misid/rk/electron/{sample}.yaml')

    with Cache.cache_root(path = tmp_path):
        ftr = SimFitter(
            name    = 'test_weights',
            sample  = sample,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_nopid,
            q2bin   = 'central')
        ftr.get_model()
# ---------------------------------------------------
