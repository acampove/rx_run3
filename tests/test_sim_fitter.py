'''
This module is meant to test the SimFitter class
'''
import pytest

from dmu.stats.zfit         import zfit
from dmu.generic            import utilities as gut
from dmu.stats              import utilities as sut
from dmu.workflow.cache     import Cache
from rx_common.types        import Trigger
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection as sel
from fitter.sim_fitter      import SimFitter

# ---------------------------------------------------
def test_nomc():
    '''
    Test for components that have no MC associated
    '''
    obs = zfit.Space('B_Mass', limits=(4500, 7000))

    cfg = gut.load_conf(package='fitter_data', fpath='rare/rk/electron/combinatorial.yaml')
    ftr = SimFitter(
        component= 'combinatorial',
        obs     = obs,
        cfg     = cfg,
        trigger = Trigger.rk_ee_os,
        q2bin   = 'low')
    _ = ftr.get_model()
# ---------------------------------------------------
def test_nocat():
    '''
    Test for components without categories, e.g. muon
    '''
    block = 1
    obs   = zfit.Space('B_Mass', limits=(5000, 5800))
    cfg   = gut.load_conf(package='fitter_data', fpath='rare/rk/muon/signal.yaml')

    with RDFGetter.max_entries(value=-1),\
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):

        ftr = SimFitter(
            component= 'signal_muon',
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_mm_os,
            q2bin   = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
def test_with_cat():
    '''
    Test for components with brem categories
    '''
    block = 1
    obs   = zfit.Space('B_Mass', limits=(4500, 7000))
    cfg   = gut.load_conf(package='fitter_data', fpath='rare/rk/electron/signal_parametric.yaml')

    with RDFGetter.max_entries(value=-1),\
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):

        ftr = SimFitter(
            component= 'signal_electron',
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'jpsi')
        _ = ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['bdkstee', 'bukstee', 'bsphiee'])
def test_kde(component : str):
    '''
    Test fitting with KDE
    '''
    block = 1
    obs   = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{component}.yaml')

    with Cache.turn_off_cache(val = ['SimFitter', 'DataPreprocessor']),\
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):

        ftr = SimFitter(
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'central')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['kkk', 'kpipi'])
@pytest.mark.parametrize('q2bin'    , ['low', 'central', 'high'])
def test_misid(component : str, q2bin : str):
    '''
    Test fitting misID simulation 
    '''
    block = 1
    obs   = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{component}.yaml')

    # For no PID triggers, take unsmeared simulation. This simulation will be B -> 3/4 H and
    # smearing is not needed
    with RDFGetter.multithreading(nthreads=8),\
        RDFGetter.custom_columns(columns={'B_Mass_smr' : 'B_M', 'q2_smr' : 'q2'}),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):

        ftr = SimFitter(
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_nopid,
            q2bin    = q2bin)
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('limits', ['wide', 'narrow'])
def test_ccbar_reso(limits : str):
    '''
    Tests retriveval of PDF associated to ccbar inclusive decays
    '''
    block     = 1
    tp_limits = {'wide' : (4500, 6000), 'narrow' : (5000, 6000)}[limits]
    component = 'ccbar'
    obs       = zfit.Space('B_const_mass_M', limits=tp_limits)
    cfg       = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{component}.yaml')

    out_dir   = f'{cfg.output_directory}/{limits}'
    cfg.output_directory = out_dir
    with RDFGetter.max_entries(value=-1),\
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):

        ftr = SimFitter(
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'jpsi')
        pdf = ftr.get_model()

    assert pdf is not None

    sut.print_pdf(pdf)
# ---------------------------------------------------
def test_ccbar_rare():
    '''
    Tests retriveval of PDF associated to ccbar inclusive decays
    for rare modes, i.e. without jpsi mass constraint
    '''
    block     = 1
    component = 'ccbar'
    mass      = 'B_Mass'
    q2bin     = 'high'
    obs       = zfit.Space(mass, limits=(4500, 6000))
    cfg       = gut.load_conf(package='fitter_data', fpath=f'rare/rk/electron/{component}.yaml')

    with Cache.turn_off_cache(val=['SimFitter']),\
        sel.custom_selection(d_sel={
            'block' :f'block == {block}',
            'nobr0' : 'nbrem != 0',
            'bdt'   : 'mva_cmb > 0.8 && mva_prc > 0.8'}):
        ftr = SimFitter(
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = q2bin)
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['signal', 'cabibbo'])
@pytest.mark.parametrize('brem'     , [1, 2])
def test_mc_reso(component : str, brem : int):
    '''
    Tests retriveval of PDF associated to ccbar inclusive decays
    '''
    block = 1
    obs   = zfit.Space('B_const_mass_M', limits=(5000, 6900))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{component}.yaml')

    with RDFGetter.max_entries(value=-1),\
        RDFGetter.multithreading(nthreads=8),\
        Cache.turn_off_cache(val=None), \
        sel.custom_selection(d_sel={
            'block' :f'block == {block}',
            'mass'  : '(1)',
            'nbrem' :f'nbrem == {brem}'}):
        ftr = SimFitter(
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('name', ['name_001', 'name_002'])
def test_name(name : str):
    '''
    Will run test and specify the name argument
    '''
    block     = 1
    component = 'ccbar'
    obs       = zfit.Space('B_const_mass_M', limits=(4500, 6000))
    cfg       = gut.load_conf(package='fitter_data', fpath=f'reso/rk/electron/{component}.yaml')

    with RDFGetter.max_entries(value=-1),\
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):
        ftr = SimFitter(
            name     = name,
            component= component,
            obs      = obs,
            cfg      = cfg,
            trigger  = Trigger.rk_ee_os,
            q2bin    = 'jpsi')
        ftr.get_model()
# ---------------------------------------------------
@pytest.mark.parametrize('component', ['kpipi', 'kkk'])
def test_weights(component : str):
    '''
    Test fitting weighted MC sample
    '''
    block = 1
    obs   = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg   = gut.load_conf(package='fitter_data', fpath=f'misid/rk/electron/{component}.yaml')

    with RDFGetter.max_entries(value=-1),\
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel = {'block' : f'block == {block}'}):
        ftr = SimFitter(
            component= component,
            obs     = obs,
            cfg     = cfg,
            trigger = Trigger.rk_ee_nopid,
            q2bin   = 'central')
        ftr.get_model()
